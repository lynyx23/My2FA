#include "LogoutRequestCommand.hpp"
#include <sstream>
#ifdef SERVER_SIDE
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#endif

LogoutRequestCommand::LogoutRequestCommand(std::string uuid)
    : m_uuid(std::move(uuid)) {}

std::string LogoutRequestCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::LOGOUT_REQ) << DELIMITER << m_uuid;
    return ss.str();
}

void LogoutRequestCommand::execute(Context &ctx, int client_fd) {
#ifdef SERVER_SIDE
    ctx.session_manager.logout(client_fd);
    std::cout << "[Server] User logged out: " << m_uuid << "\n";
#endif
}

CommandType LogoutRequestCommand::getType() const {
    return CommandType::LOGOUT_REQ;
}

std::string LogoutRequestCommand::getUuid() const {
    return m_uuid;
}
