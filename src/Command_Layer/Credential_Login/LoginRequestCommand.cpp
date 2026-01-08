#include <iostream>
#include <sstream>
#include "LoginRequestCommand.hpp"
#include "Command_Layer/Context.hpp"

#ifdef SERVER_SIDE
#include <string>
#include "Auth_Layer/AuthManager.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Command_Layer/Credential_Login/LoginResponseCommand.hpp"
#include "Command_Layer/System_Commands/ErrorCommand.hpp"
#endif

LoginRequestCommand::LoginRequestCommand(std::string user, std::string pass)
    : m_username(std::move(user)), m_password(std::move(pass)) {}

std::string LoginRequestCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::LOGIN_REQ) << DELIMITER << m_username << DELIMITER << m_password;
    return ss.str();
}

void LoginRequestCommand::execute(Context &ctx, const int fd) {
#ifdef SERVER_SIDE
    if (ctx.session_manager.getIsLogged(fd)) {
        ctx.server_handler.sendCommand(fd,
            std::make_unique<ErrorCommand>(300,"User already logged in!"));
        std::cerr << "[AM Error] User already logged in!\n";
        return;
    }
    bool resp;
    std::string uuid;
    if (ctx.auth_manager->loginUser(m_username,m_password)) {
        ctx.session_manager.setIsLogged(fd, true);
        std::cout << "[AM Log] Login successful: "
            << m_username <<" (fd = " << fd << ")\n";
        resp = true;
        uuid = ctx.auth_manager->getUUID(m_username);
        ctx.session_manager.setUUID(fd, uuid);
        ctx.session_manager.setSecret(fd, ctx.auth_manager->getSecret(uuid));
    }
    else {
        ctx.session_manager.setIsLogged(fd, false);
        std::cerr << "[AM Error] Login failed: "
            << m_username <<" (fd = " << fd << ")\n";
        resp = false;
        uuid = "0";
        ctx.server_handler.sendCommand(fd,
            std::make_unique<ErrorCommand>(301,"Invalid username or password!"));
    }
    ctx.server_handler.sendCommand(fd,
        std::make_unique<LoginResponseCommand>(resp, uuid));
    std::cout << "[Server] Sending Login Response to Client: " << fd << "\n";
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
