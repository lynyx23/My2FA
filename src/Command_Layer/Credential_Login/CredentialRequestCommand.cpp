#include "CredentialRequestCommand.hpp"
#include <sstream>
#include <string>
#include <utility>
#if defined(A_SERVER) || defined(D_SERVER)
#include "Auth_Layer/AuthManager.hpp"
#include "Command_Layer/Context.hpp"
#include "Command_Layer/System_Commands/ErrorCommand.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Command_Layer/System_Commands/GenericResponseCommand.hpp"
#include "Database_Layer/Database.hpp"
#endif

CredentialRequestCommand::CredentialRequestCommand(const CommandType type, std::string user, std::string pass):
    m_type(type), m_username(std::move(user)), m_password(std::move(pass)) {}

std::string CredentialRequestCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::CRED_REQ) << DELIMITER << static_cast<int>(m_type)
        << DELIMITER << m_username << DELIMITER << m_password;
    return ss.str();
}

void CredentialRequestCommand::execute(Context &ctx, const int fd) {
#if defined(A_SERVER) || defined(D_SERVER)
    bool resp = false;
    CommandType resp_type;
    //std::cout << "[DEBUG] type " << m_type << " | user " << m_username << " | pass " << m_password << "\n";
    if (m_type == CommandType::LOGIN_REQ) {
        if (ctx.session_manager.getIsLogged(fd)) {
            ctx.server_handler.sendCommand(fd,
                std::make_unique<ErrorCommand>(300,"User already logged in!"));
            std::cerr << "[AM Error] User already logged in!\n";
            return;
        }
        if (ctx.auth_manager->loginUser(m_username,m_password)) {
            ctx.session_manager.setIsLogged(fd, true);
            std::cout << "[AM Log] Login successful: "
                << m_username <<" (fd = " << fd << ")\n";
            resp = true;
            resp_type = CommandType::LOGIN_RESP;
            ctx.session_manager.setIdentity(fd, m_username);
#ifdef A_SERVER
            ctx.session_manager.setSecretPairings(fd, Database::getSecretPairings(m_username));
#endif
        }
        else {
            ctx.session_manager.setIsLogged(fd, false);
            std::cerr << "[AM Error] Login failed: "
                << m_username <<" (fd = " << fd << ")\n";
            ctx.server_handler.sendCommand(fd,
                std::make_unique<ErrorCommand>(301,"Invalid username or password!"));
        }
    }

    else if (m_type == CommandType::REGISTER_REQ) {
        if (ctx.session_manager.getIsLogged(fd)) {
            ctx.server_handler.sendCommand(fd,
                std::make_unique<ErrorCommand>(300,"User already logged in!"));
            std::cerr << "[AM Error] User already logged in!\n";
            return;
        }

        if (ctx.auth_manager->registerUser(m_username, m_password)) {
            //std::cout << "[AM Log] User registered: " << m_username << "\n";
            resp = true;
            resp_type = CommandType::REGISTER_RESP;
        } else {
            //std::cerr << "[AM Error] Adding user failed: " << e.what() << "\n";
            ctx.server_handler.sendCommand(fd,
                std::make_unique<ErrorCommand>(302,"Username already taken!"));
        }
    }

    if (resp) {
        ctx.server_handler.sendCommand(fd,
            std::make_unique<GenericResponseCommand>(resp_type, resp, "", m_username));
        std::cout << "[Server] Sending Credential Command Response to Client: " << fd << "\n";
    }
#endif
}

CommandType CredentialRequestCommand::getType() const { return m_type; }

std::string CredentialRequestCommand::getUsername() const { return m_username; }

std::string CredentialRequestCommand::getPassword() const { return m_password; }
