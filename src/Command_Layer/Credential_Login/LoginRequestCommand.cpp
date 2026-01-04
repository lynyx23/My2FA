#include <iostream>
#include <sstream>
#include "LoginRequestCommand.hpp"

#ifdef SERVER_SIDE
#include "Command_Layer/ServerContext.hpp"
#include "Auth_Layer/AuthManager.hpp"
#include "Session_Manager/SessionManager.hpp"
#endif

LoginRequestCommand::LoginRequestCommand(std::string user, std::string pass)
    : m_username(std::move(user)), m_password(std::move(pass)) {}

std::string LoginRequestCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::LOGIN_REQ) << DELIMITER << m_username << DELIMITER << m_password;
    return ss.str();
}

void LoginRequestCommand::execute(ServerContext &ctx, const int client_fd) {
#ifdef SERVER_SIDE
    if (ctx.auth_manager->loginUser(m_username,m_password)) {
        ctx.session_manager.setIsLogged(client_fd, true);
        std::cout << "[AM Log] Login successful: "
            << m_username <<" (fd = " << client_fd << ")\n";
    }
    else {
        ctx.session_manager.setIsLogged(client_fd, false);
        std::cout << "[AM Log] Login failed: "
            << m_username <<" (fd = " << client_fd << ")\n";
    }
#endif
}

std::string LoginRequestCommand::getPassword() const {
    return m_password;
}

std::string LoginRequestCommand::getUsername() const {
    return m_username;
}

CommandType LoginRequestCommand::getType() const  {
    return CommandType::LOGIN_REQ;
}
