#include "ExitSCSCommand.hpp"

#include "Session_Manager/SessionManager.hpp"
#ifdef A_SERVER
#include "Command_Layer/Context.hpp"
#endif
std::string ExitSCSCommand::serialize() const {
    return std::to_string(static_cast<int>(CommandType::EXIT_SCS));
}

void ExitSCSCommand::execute(Context &ctx, const int client_fd) {
#ifdef A_SERVER
    ctx.session_manager.setIsInCodeState(client_fd, false);
    std::cout << "[Server] Client " << client_fd << " exited ShowCode State!\n";
#endif
}

CommandType ExitSCSCommand::getType() const {
    return CommandType::EXIT_SCS;
}
