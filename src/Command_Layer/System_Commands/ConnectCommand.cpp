#include <sstream>
#include "ConnectCommand.hpp"

#ifdef SERVER_SIDE
#include "Session_Manager/SessionManager.hpp"
#include "Command_Layer/ServerContext.hpp"
#endif

ConnectCommand::ConnectCommand(EntityType type): m_connection_type(std::move(type)) {}

std::string ConnectCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::CONN) << DELIMITER
        << static_cast<int>(m_connection_type);
    return ss.str();
}

void ConnectCommand::execute(ServerContext &ctx, int client_fd) {
#ifdef SERVER_SIDE
    ctx.session_manager.m_handleHandshake(client_fd, m_connection_type);
#endif
}

CommandType ConnectCommand::getType() const {
    return CommandType::CONN;
}

EntityType ConnectCommand::getConnectionType() const {
    return m_connection_type;
}

