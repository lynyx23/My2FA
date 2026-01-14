#include "LogoutRequestCommand.hpp"
#include <sstream>

#include "Command_Layer/System_Commands/ErrorCommand.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#if defined(A_SERVER) || defined(D_SERVER)
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#endif

std::string LogoutRequestCommand::serialize() const {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::LOGOUT_REQ);
    return ss.str();
}

void LogoutRequestCommand::execute(Context &ctx, int client_fd) {
#if defined(A_SERVER) || defined(D_SERVER)
    if (ctx.session_manager.getIsLogged(client_fd)) {
        std::cout << "[Server] User logged out: " << ctx.session_manager.getIdentity(client_fd) << "\n";
        ctx.session_manager.logout(client_fd);
    }
    else {
        ctx.server_handler.sendCommand(client_fd,
            std::make_unique<ErrorCommand>(303,"User not logged in!"));
    }
#endif
}

CommandType LogoutRequestCommand::getType() const {
    return CommandType::LOGOUT_REQ;
}
