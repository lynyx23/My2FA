#include "SendNotificationCommand.hpp"
#include <sstream>
#include <map>
#ifdef A_CLIENT
#include "Command_Layer/Context.hpp"
#endif

SendNotificationCommand::SendNotificationCommand(std::string reqID, std::string app_id):
    m_req_id(std::move(reqID)), m_app_id(std::move(app_id)){  }

std::string SendNotificationCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::SEND_NOTIF) << DELIMITER << m_req_id << DELIMITER << m_app_id;
    return ss.str();
}

void SendNotificationCommand::execute(Context &ctx, const int client_fd) {
#ifdef A_CLIENT
    //avoid duplicates
    for (const auto &[reqID, appID] : ctx.pendingNotifications) {
        if (reqID == m_req_id) return;
    }

    ctx.pendingNotifications.push_back({m_req_id, m_app_id});

    if (!ctx.codeState) {
        int idx = ctx.pendingNotifications.size();
        std::cout << "\n[!] Notification #" << idx << ": " << m_app_id << "\n"
                  << "    Type accept or refuse #!\n" << std::flush;
    }
    else std::cout << "\a" << std::flush;
#endif
}

CommandType SendNotificationCommand::getType() const {
    return CommandType::SEND_NOTIF;
}

