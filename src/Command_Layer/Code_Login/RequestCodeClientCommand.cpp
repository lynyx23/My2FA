#include "RequestCodeClientCommand.hpp"
#include <sstream>
#include <utility>

#include "Session_Manager/SessionManager.hpp"
#include "TOTP_Layer/TOTPManager.hpp"
#ifdef SERVER_SIDE
#include "Command_Layer/Context.hpp"

#endif

//TODO IMPLEMENT APPID
RequestCodeClientCommand::RequestCodeClientCommand(std::string uuid, const int appid)
        : m_uuid(std::move(uuid)), m_appid(appid) {}

std::string RequestCodeClientCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::REQ_CODE_CLIENT) << DELIMITER << m_uuid << DELIMITER << m_appid;
    return ss.str();
}

void RequestCodeClientCommand::execute(Context &ctx, int client_fd) {
#ifdef SERVER_SIDE
    auto session = ctx.session_manager.getSession((client_fd));
    if (!session) {
        std::cerr << "[SM Log] Session not found! (" << client_fd << ")\n";
        return;
    }
    ctx.session_manager.setIsInCodeState(client_fd, true);
    ctx.totp_manager->sendCodeToClient(session);
#endif
};

CommandType RequestCodeClientCommand::getType() const {
    return CommandType::REQ_CODE_CLIENT;
}

std::string RequestCodeClientCommand::getUuid() const {
    return m_uuid;
}

int RequestCodeClientCommand::getAppid() const {
    return m_appid;
}