#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "../Connection_Layer/ClientConnectionHandler.hpp"

#define PORT 27701

void handleUserInput(ClientConnectionHandler &handler, const std::string &input) {
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
                << "  exit                   : Quit\n";
        return;
    } else if (args[0] == "exit") {
        handler.disconnect();
        exit(0);
    } else if (args[0] == "conn") {
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
        handler.sendCommand(command);
        std::cout << "[AC Sent] " << data << "\n";
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
    ClientConnectionHandler handler("127.0.0.1", PORT);
    handler.setCallback([&](const std::unique_ptr<Command> &command) {
        std::cout << "[AC Log] Handling command ...\n" << command->serialize() << "\n";
    });

    while (true) {
        handler.update();

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                handleUserInput(handler, input);
            }
        }
    }
}
