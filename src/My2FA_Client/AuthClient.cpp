#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "../Connection_Layer/ClientConnectionHandler.hpp"

#define PORT 27701

void handleUserInput(ClientConnectionHandler *handler, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;
    // note: switch won't work with strings
    if (args[0] == "help") {
        std::cout << "  help                   : Show this menu\n"
                << "  conn <id>              : Handshake (e.g. conn;12)\n"
                << "  code <uuid> <appid>    : Request 2FA Code (e.g. code;uuid_123;101)\n"
                << "  accept <appid>         : Accept Notification (e.g. accept;101)\n"
                << "  refuse <appid>         : Refuse Notification (e.g. refuse;101)\n"
                << "  reconnect              : Reconnect to AS\n"
                << "  exit                   : Quit\n";
        return;
    } else if (args[0] == "reconnect") return;
    else if (args[0] == "conn") {
        if (args.size() != 2) {
            std::cerr << "[AC Error] Incorrect format: conn <id>\n ";
            return;
        }
        command = std::make_unique<ConnectCommand>("AUTH_CLIENT", args[1]);
    } else if (args[0] == "accept") {
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

std::string handleCommand(const std::unique_ptr<Command> &command) {
    std::ostringstream ss;
    switch (command->getType()) {
        case CommandType::ERR: {
            const auto *cmd = dynamic_cast<const ErrorCommand *>(command.get());
            ss << "Received command ERR: Type = " << cmd->getCode()
                    << " , Msg = " << cmd->getMessage();
            break;
        }
        case CommandType::SEND_NOTIF: {
            const auto *cmd = dynamic_cast<const SendNotificationCommand *>(command.get());
            ss << "Received command SEND_NOTIF: AppID = " << cmd->getAppid();
            break;
        }
        default:
            ss << "Invalid or Unexpected command type: "
                    << static_cast<int>(command->getType());
    }
    return ss.str();
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
    std::unique_ptr<ClientConnectionHandler> handler = nullptr;
    try {
        handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", PORT);
        handler->setCallback([&](const std::unique_ptr<Command> &command) {
            std::cout << "[AC Log] Handling command ...\n" << handleCommand(command) << "\n";
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
            if (split(input)[0] == "exit") {
                run = false;
            }
            if (split(input)[0] == "reconnect") {
                try {
                    handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", PORT);
                    handler->setCallback([&](const std::unique_ptr<Command> &command) {
                        std::cout << "[AC Log] Handling command ...\n"
                                << handleCommand(command) << "\n";
                    });
                    is_connected = true;
                } catch (...) {
                    is_connected = false;
                    std::cerr << "[AC Error] Connection to AS Failed." << "\n";
                }
            }
            if (!input.empty()) {
                handleUserInput(handler.get(), input);
            }
        }
    }
}