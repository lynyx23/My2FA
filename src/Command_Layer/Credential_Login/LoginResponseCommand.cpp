#include "LoginResponseCommand.hpp"
#include <iostream>
#include <sstream>
#include "Command_Layer/Context.hpp"

LoginResponseCommand::LoginResponseCommand(const bool resp, std::string uuid) :
    m_response(resp), m_uuid(std::move(uuid)) {}

std::string LoginResponseCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::LOGIN_RESP) << DELIMITER << static_cast<int>(m_response) << DELIMITER << m_uuid;
    return ss.str();
}

void LoginResponseCommand::execute(Context &ctx, const int client_fd) {
#ifdef SERVER_SIDE
#else
    if (m_response) {
        ctx.isLogged = true;
        ctx.uuid = m_uuid;
        std::cout << "Login successful: " << m_uuid << "\n";
    } else {
        ctx.isLogged = false;
        std::cout << "Login failed: " << m_uuid << "\n";
    }
#endif
}

CommandType LoginResponseCommand::getType() const { return CommandType::LOGIN_RESP; }

bool LoginResponseCommand::getResponse() const { return m_response; }

std::string LoginResponseCommand::getUuid() const { return m_uuid; }
