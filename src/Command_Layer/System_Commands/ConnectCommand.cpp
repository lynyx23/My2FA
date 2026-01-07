#include <sstream>
#include "ConnectCommand.hpp"

#ifdef SERVER_SIDE
#include "Session_Manager/SessionManager.hpp"
#include "Command_Layer/Context.hpp"
#endif

ConnectCommand::ConnectCommand(const EntityType type): m_connection_type(type) {}

std::string ConnectCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::CONN) << DELIMITER
        << static_cast<int>(m_connection_type);
    return ss.str();
}

void ConnectCommand::execute(Context &ctx, int fd) {
#ifdef SERVER_SIDE
    ctx.session_manager.handleHandshake(fd, m_connection_type);
#endif
}

CommandType ConnectCommand::getType() const {
    return CommandType::CONN;
}

EntityType ConnectCommand::getConnectionType() const {
    return m_connection_type;
}

