#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Context.hpp"
#include "Command_Layer/Credential_Login/LoginRequestCommand.hpp"
#include "Command_Layer/Credential_Login/LogoutRequestCommand.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/System_Commands/SystemCommands.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"

#define PORT 27701

void handleUserInput(Context &ctx, ClientConnectionHandler *handler, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;
    // note: switch won't work with strings
    if (args[0] == "help") {
        std::cout << "  help                   : Show this menu\n"
                << "  conn                   : Handshake (e.g. conn)\n"
                << "  code <uuid> <appid>    : Request 2FA Code (e.g. code;uuid_123;101)\n"
                << "  accept <appid>         : Accept Notification (e.g. accept;101)\n"
                << "  refuse <appid>         : Refuse Notification (e.g. refuse;101)\n"
                << "  reconnect              : Reconnect to AS\n"
                << "  exit                   : Quit\n";
        return;
    } else if (args[0] == "reconnect") return;
    else if (args[0] == "conn") command = std::make_unique<ConnectCommand>(EntityType::AUTH_CLIENT);
    else if (args[0] == "accept") {
        if (args.size() < 2) {
            std::cerr << "[AC Error] Incorrect format: accept <appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(true, std::stoi(args[1]));
    } else if (args[0] == "refuse") {
        if (args.size() < 2) {
            std::cerr << "[Ac Error] Incorrect format: refuse <appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(false, std::stoi(args[1]));
    } else if (args[0] == "code") {
        if (args.size() != 3) {
            std::cerr << "[AC Error] Incorrect format: code <uuid> <appid>\n ";
            return;
        }
        try {
            command = std::make_unique<RequestCodeClientCommand>(args[1], std::stoi(args[2]));
        } catch (...) {
            std::cerr << "[AC Error] Code must be an integer.\n";
            return;
        }
    } else if (args[0] == "login") {
        if (args.size() != 3) {
            std::cerr << "[AC Error] Incorrect format: login <user> <pass>\n ";
            return;
        }
        command = std::make_unique<LoginRequestCommand>(args[1], args[2]);
    } else if (args[0] == "logout") {
        command = std::make_unique<LogoutRequestCommand>(ctx.uuid);
    } else {
        std::cout << "[AC Error] Unknown command! Type help.\n ";
        return;
    }

    if (command) {
        const std::string data = command->serialize();
        if (handler && handler->isRunning()) {
            handler->sendCommand(command);
            std::cout << "[AC -> AS] Sent: " << data << "\n";
        } else std::cerr << "[AC Error] Not connected to DS.\n";
    }
}

void handleCommand(Context &ctx, ClientConnectionHandler *handler,
    const std::unique_ptr<Command> &command, const int fd) {
    if (!command) {
        std::cerr << "[AC Error] Received invalid command from server.\n";
        return;
    }
    try {
        command->execute(ctx, fd);
    } catch (const std::exception &e) {
        std::cerr << "[AC Error] Command execution failed: " << e.what() << "\n";
    }
    // std::ostringstream ss;
    // switch (command->getType()) {
    //     case CommandType::ERR: {
    //         const auto *cmd = dynamic_cast<const ErrorCommand *>(command.get());
    //         ss << "Received command ERR: Type = " << cmd->getCode()
    //                 << " , Msg = " << cmd->getMessage();
    //         break;
    //     }
    //     case CommandType::SEND_NOTIF: {
    //         const auto *cmd = dynamic_cast<const SendNotificationCommand *>(command.get());
    //         ss << "Received command SEND_NOTIF: AppID = " << cmd->getAppid();
    //         break;
    //     }
    //     default:
    //         ss << "Invalid or Unexpected command type: "
    //                 << static_cast<int>(command->getType());
    // }
    // return ss.str();

}

bool checkConsoleInput() {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    timeval timeout{0, 0};
    return select(STDIN_FILENO + 1, &read_set, nullptr, nullptr, &timeout) > 0;
}

int main() {
    bool is_connected;

    Context ctx{false, "0"};

    std::unique_ptr<ClientConnectionHandler> handler = nullptr;
    try {
        handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", PORT);
        handler->setCallback([&](const int fd, const std::unique_ptr<Command> &command) {
            std::cout << "[AC Log] Handling command ...\n";
            handleCommand(ctx, handler.get(), command, fd);
        });
        is_connected = true;
    } catch (...) {
        is_connected = false;
        std::cerr << "[AC Error] Connection to AS Failed." << "\n";
    }

    bool run = true;
    while (run) {
        if (is_connected && handler) {
            if (!handler->isRunning()) {
                std::cerr << "[AC Error] AS disconnected!\n";
                is_connected = false;
            } else handler->update();
        }

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (split(input)[0] == "exit") {
                    run = false;
                }
                if (split(input)[0] == "reconnect") {
                    try {
                        handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", PORT);
                        handler->setCallback([&](const int fd, const std::unique_ptr<Command> &command) {
                            std::cout << "[AC Log] Handling command ...\n";
                            handleCommand(ctx, handler.get(), command, fd);
                        });
                        is_connected = true;
                    } catch (...) {
                        is_connected = false;
                        std::cerr << "[AC Error] Connection to AS Failed." << "\n";
                    }
                }
                handleUserInput(ctx, handler.get(), input);
            }
        }
    }
}