#include "RequestNotificationCommand.hpp"
#include <sstream>

#include "Auth_Layer/AuthManager.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "SendNotificationCommand.hpp"
#ifdef D_SERVER
#include "Command_Layer/Context.hpp"
#elif defined(A_SERVER)
#include "Command_Layer/Context.hpp"
#include "Database_Layer/Database.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"
#endif

RequestNotificationCommand::RequestNotificationCommand(std::string username, std::string app_id):
    m_username(std::move(username)), m_app_id(std::move(app_id)){  }

std::string RequestNotificationCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::REQ_NOTIF) << DELIMITER << m_username << DELIMITER << m_app_id;
    return ss.str();
}

void RequestNotificationCommand::execute(Context &ctx, const int client_fd) {
#ifdef D_SERVER
    ctx.client_handler->sendCommand(
        std::make_unique<RequestNotificationCommand>(m_username, ctx.app_id));
#elif defined(A_SERVER)
    std::string reqID;
    if (const int ac_fd = ctx.auth_manager->startNotification(
        m_username, m_app_id, reqID, client_fd, ctx.session_manager); ac_fd > 0) {
        ctx.server_handler.sendCommand(ac_fd,
            std::make_unique<SendNotificationCommand>(reqID, m_app_id));
        std::cout << "[AS Log] Sending Notification to Client: " << ac_fd << "\n";
    }
#endif
}

CommandType RequestNotificationCommand::getType() const {
    return CommandType::REQ_NOTIF;
}
