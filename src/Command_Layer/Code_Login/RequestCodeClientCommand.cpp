#include "RequestCodeClientCommand.hpp"
#ifdef A_SERVER
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "TOTP_Layer/TOTPManager.hpp"
#endif

std::string RequestCodeClientCommand::serialize() const {
    return std::to_string(static_cast<int>(CommandType::REQ_CODE_CLIENT));
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