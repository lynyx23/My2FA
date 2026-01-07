#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Auth_Layer/AuthManager.hpp"
#include "Command_Layer/Context.hpp"

#define PORT 27701 // Auth port
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

void handleCommand(const std::unique_ptr<Command> &command, const int client_fd, Context &ctx) {
    if (!command) {
        std::cerr << "[AS Error] Received invalid command from client " << client_fd << "\n";
        return;
    }
    try {
        command->execute(ctx, client_fd);
    } catch (const std::exception &e) {
        std::cerr << "[AS Error] Command " << "(" << command->getType() << ") execution failed: " << e.what() << "\n";
    }
        // case CommandType::REQ_NOTIF_SERVER: {
        //     const auto *cmd = dynamic_cast<const RequestNotificationServerCommand *>(command.get());
        //     ss << "Received command REQ_NOTIF_SERVER: UUID = " << cmd->getUuid()
        //             << " , AppID = " << cmd->getAppid();
        //     break;
        // }
        // case CommandType::NOTIF_RESP_CLIENT: {
        //     const auto *cmd = dynamic_cast<const NotificationResponseClientCommand *>(command.get());
        //     ss << "Received command NOTIF_RESP_CLIENT: Response = "
        //             << (cmd->getResponse() ? "True" : "False")
        //             << " , AppID = " << cmd->getAppid();
        //     break;
        // }
        // case CommandType::REQ_CODE_CLIENT: {
        //     const auto *cmd = dynamic_cast<const RequestCodeClientCommand *>(command.get());
        //     ss << "Received command REQ_CODE_CLIENT: UUID = " << cmd->getUuid()
        //             << " , AppID = " << cmd->getAppid();
        //     break;
        // }
        // case CommandType::VALIDATE_CODE_SERVER: {
        //     const auto *cmd = dynamic_cast<const ValidateCodeServerCommand *>(command.get());
        //     ss << "Received command VALIDATE_CODE_SERVER: Code = " << cmd->getCode()
        //             << " , UUID = " << cmd->getUuid()
        //             << " , AppID = " << cmd->getAppid();
        //     break;
        // }
}

void handleUserInput(ServerConnectionHandler &handler, const std::string &input, SessionManager &session_manager) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << "  clients                                     : List all active client file descriptors.\n"
                << "  send_notif <appid>                          : Send Push to Client (e.g. send_notif;12)\n"
                << "  code_resp <code>                            : Send Code to Client (e.g. code_resp;123456)\n"
                << "  notif_resp_server <1/0> <uuid>              : Send Push Result to DS (e.g. notif_resp_server;1;user123)\n"
                << "  validate_resp_server <1/0> <uuid> <appid>   : Send Code Result to DS (e.g. validate_resp_server;1;user123;12)\n"
                << "  exit                                        : Shut down the server.\n";
        return;
    } else if (args[0] == "clients") {
        session_manager.displayConnections();
    } else if (args[0] == "exit") {
        std::cout << "[AS Log] Shutting down...\n";
        exit(0);
    } else if (args[0] == "send_notif") {
        if (args.size() < 2) {
            std::cerr << "[AS Error] Usage: send_notif <appid>\n";
            return;
        }
        command = std::make_unique<SendNotificationCommand>(std::stoi(args[1]));
    } else if (args[0] == "code_resp") {
        if (args.size() < 2) {
            std::cerr << "[AS Error] Usage: code_resp <code>\n";
            return;
        }
        command = std::make_unique<CodeResponseCommand>(std::stoi(args[1]));
    } else if (args[0] == "notif_resp_server") {
        if (args.size() < 3) {
            std::cerr << "[AS Error] Usage: notif_resp_server <1/0> <uuid>\n";
            return;
        }
        bool resp = (args[1] == "1" || args[1] == "true");
        command = std::make_unique<NotificationResponseServerCommand>(resp, args[2]);
    } else if (args[0] == "validate_resp_server") {
        if (args.size() < 4) {
            std::cerr << "[AS Error] Usage: validate_resp_server <1/0> <uuid> <appid>\n";
            return;
        }
        bool resp = (args[1] == "1" || args[1] == "true");
        command = std::make_unique<ValidateResponseServerCommand>(resp, args[2], std::stoi(args[3]));
    } else {
        std::cerr << "[AS Error] Unknown command. Type 'help'.\n";
        return;
    }

    if (command) {
        const std::string data = command->serialize();

        switch (command->getType()) {
            case CommandType::SEND_NOTIF:
            case CommandType::CODE_RESP:
                std::cout << "[AS -> AC] Sending: " << data << "\n";
                handler.broadcastCommand(command);
                break;

            case CommandType::NOTIF_RESP_SERVER:
            case CommandType::VALIDATE_RESP_SERVER:
                std::cout << "[AS -> DS] Sending: " << data << "\n";
                handler.broadcastCommand(command);
                break;

            default:
                std::cerr << "[AS Error] No routing rule defined for Command ID "
                        << static_cast<int>(command->getType()) << "\n";
                break;
        }
    }
}

bool checkConsoleInput() {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    timeval timeout{0, 0};
    return select(STDIN_FILENO + 1, &read_set, nullptr, nullptr, &timeout) > 0;
}

int main() {
    signal(SIGPIPE, SIG_IGN); // avoid crashes from sending

    auto auth_manager = std::make_unique<AuthManager>("as");

    SessionManager session_manager;

    ServerConnectionHandler handler(PORT);

    Context ctx{session_manager, auth_manager.get(), handler};

    handler.setCommandCallback([&](const int client_fd, const std::unique_ptr<Command> &command) {
        std::cout << "[AS Log] Handling command from Client: " << client_fd << " ("
            << ctx.session_manager.getEntityType(client_fd) << ")\n";
        handleCommand(command, client_fd, ctx);
    });
    handler.setConnectCallback([&](const int client_fd) {
        session_manager.addSession(client_fd);
        std::cout << "[AS Log] New client connected: " << client_fd << "\n";
    });
    handler.setDisconnectCallback([&](const int client_fd) {
        session_manager.removeSession(client_fd);
        std::cout << "[AS Log] Client disconnected: " << client_fd << "\n";
    });

    bool run = true;
    while (run) {
        handler.update();

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (split(input)[0] == "exit") run = false;
                handleUserInput(handler, input, session_manager);
            }
        }
    }
}
