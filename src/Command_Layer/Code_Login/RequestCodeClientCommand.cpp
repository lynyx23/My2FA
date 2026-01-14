#include "RequestCodeClientCommand.hpp"
#include <sstream>
#ifdef A_SERVER
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "TOTP_Layer/TOTPManager.hpp"
#endif

RequestCodeClientCommand::RequestCodeClientCommand(std::string appid)
        : m_appid(std::move(appid)) {}

std::string RequestCodeClientCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::REQ_CODE_CLIENT) << DELIMITER << m_appid;
    return ss.str();
}

void RequestCodeClientCommand::execute(Context &ctx, int client_fd) {
#ifdef A_SERVER
    auto session = ctx.session_manager.getSession((client_fd));
    if (!session) {
        std::cerr << "[SM Log] Session not found! (" << client_fd << ")\n";
        return;
    }
    ctx.session_manager.setIsInCodeState(client_fd, true);
    ctx.totp_manager->sendCodesToClient(session);
#endif
};

CommandType RequestCodeClientCommand::getType() const {
    return CommandType::REQ_CODE_CLIENT;
}

std::string RequestCodeClientCommand::getAppid() const {
    return m_appid;
}