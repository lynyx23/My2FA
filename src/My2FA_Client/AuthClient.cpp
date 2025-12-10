#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"

#define PORT 27701

void handleUserInput(int socket, const std::string &input) {
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
        exit(0);
    } else if (args[0] == "conn") {
        if (args.size() != 2) {
            std::cout << "[AC Error] Incorrect format: conn <id>\n ";
            return;
        }
        command = std::make_unique<ConnectCommand>("AUTH_CLIENT", args[1]);
    } else if (args[0] == "accept") {
        if (args.size() < 2) {
            std::cout << "[AC Error] Incorrect format: accept <appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(static_cast<bool>(1), std::stoi(args[1]));
    } else if (args[0] == "refuse") {
        if (args.size() < 2) {
            std::cout << "[Ac Error] Incorrect format: refuse <appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(false, std::stoi(args[1]));
    } else if (args[0] == "code") {
        if (args.size() != 3) {
            std::cout << "[AC Error] Incorrect format: code <uuid> <appid>\n ";
            return;
        }
        command = std::make_unique<RequestCodeClientCommand>(args[1], std::stoi(args[2]));
        //implement error catch for stoi
    } else {
        std::cout << "[AC Error] Unknown command! Type help.\n ";
        return;
    }

    if (command) {
        std::string data = command->serialize();
        send(socket, data.c_str(), data.length(), 0);
        std::cout << "[AC Sent] " << data << "\n";
    }
}

int main() {
    int client_socket = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[AC Error] Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("[AC Errror] Invalid address");
        return -1;
    }

    std::cout << "[AC Log] Connecting to Auth Server on port " << PORT << "...\n";
    if (connect(client_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("[AC Error] Connection Failed");
        return -1;
    }

    ConnectCommand conn("AUTH_CLIENT", "ac_01");
    std::string data = conn.serialize();
    send(client_socket, data.c_str(), data.length(), 0);

    std::cout << "[AC Log] Connected to AS and sent handshake!\n";

    fd_set read_set;
    while (true) {
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(client_socket, &read_set);

        int activity = select(client_socket + 1, &read_set, NULL, NULL, NULL);

        if (activity < 0) {
            std::cout << "[AC Error] Select error\n";
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                handleUserInput(client_socket, input);
            }
        }

        if (FD_ISSET(client_socket, &read_set)) {
            memset(buffer, 0, 1024);
            int valread = read(client_socket, buffer, 1024);
            if (valread == 0) {
                std::cout << "\n[AC Log] Server disconnected.\n";
                break;
            }

            std::string data(buffer, valread);
            std::unique_ptr<Command> command;
            try {
                command = CommandFactory::create(data);
            } catch (...) {
                command = nullptr;
            }
            if (command) {
                const auto *cmd = dynamic_cast<const SendNotificationCommand *>(command.get());
                std::stringstream ss;
                ss << "Received command SEND_NOTIF: appID =" << cmd->getAppid();
                std::cout << "[AC Log] " << ss.str() << "\n";

                send(client_socket, ss.str().c_str(), ss.str().length(), 0);
            } else {
                std::cout << "[AS Reply] " << buffer << "\n";
            }
        }
    }

    close(client_socket);
    return 0;
}
