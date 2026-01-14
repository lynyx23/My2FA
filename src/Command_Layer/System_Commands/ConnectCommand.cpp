#include <sstream>
#include "ConnectCommand.hpp"

#if defined(A_SERVER) || defined(D_SERVER)
#include "Session_Manager/SessionManager.hpp"
#include "Command_Layer/Context.hpp"
#endif

ConnectCommand::ConnectCommand(const EntityType connection_type):
    m_connection_type(connection_type) {}

ConnectCommand::ConnectCommand(const EntityType connection_type, std::string app_id):
    m_connection_type(connection_type), m_app_id(std::move(app_id)) {}

std::string ConnectCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::CONN) << DELIMITER
        << static_cast<int>(m_connection_type);
#ifdef D_SERVER
    ss << DELIMITER << m_app_id;
#endif
    return ss.str();
}

void ConnectCommand::execute(Context &ctx, const int fd) {
#if defined(A_SERVER) || defined(D_SERVER)
    ctx.session_manager.handleHandshake(fd, m_connection_type, m_app_id);
#endif
}

CommandType ConnectCommand::getType() const {
    return CommandType::CONN;
}

EntityType ConnectCommand::getConnectionType() const {
    return m_connection_type;
}

