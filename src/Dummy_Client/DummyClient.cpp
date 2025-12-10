#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"

#define DS_PORT 27702

void handleUserInput(int sock, const std::string& input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << "  help                      : Show this menu\n"
                  << "  conn <id>                 : Handshake (e.g., conn client_01)\n"
                  << "  login <user> <pass>       : Phase 1: Login (e.g., login alice 1234)\n"
                  << "  push <uuid>               : Phase 2a: Request Push (e.g., push uuid_123)\n"
                  << "  code <code> <uuid>        : Phase 2b: Enter Code (e.g., code 123456 uuid_123)\n"
                  << "  exit                      : Quit\n"
        ;
        return;
    }
    else if (args[0] == "exit") {
        close(sock);
        exit(0);
    }
    else if (args[0] == "conn") {
        if (args.size() < 2) { std::cout << "Usage: conn <id>\n> "; return; }
        command = std::make_unique<ConnectCommand>("DUMMY_CLIENT", args[1]);
    }
    else if (args[0] == "login") {
        if (args.size() < 3) { std::cout << "Usage: login <user> <pass>\n> "; return; }
        command = std::make_unique<LoginRequestCommand>(args[1], args[2]);
    }
    else if (args[0] == "push") {
        if (args.size() < 2) { std::cout << "Usage: push <uuid>\n> "; return; }
        command = std::make_unique<RequestNotificationClientCommand>(args[1]);
    }
    else if (args[0] == "code") {
        if (args.size() < 3) { std::cout << "Usage: code <code> <uuid>\n> "; return; }
        try {
            command = std::make_unique<ValidateCodeClientCommand>(std::stoi(args[1]), args[2]);
        } catch (...) {
            std::cout << "Error: Code must be an integer.\n> ";
            return;
        }
    }
    else {
        std::cout << "Unknown command. Type 'help'.\n> ";
        return;
    }

    if (command) {
        std::string data = command->serialize();
        send(sock, data.c_str(), data.length(), 0);
        std::cout << "[DC Sent] " << data << "\n> " << std::flush;
    }
}

std::string handleCommand(const std::unique_ptr<Command>& command) {
    std::ostringstream ss;
    switch(command.get()->getType()) {
        case CommandType::CONN: {
            const auto* cmd = dynamic_cast<const ConnectCommand*>(command.get());
            ss << "Received command CONN: Type = " << cmd->getConnectionType()
               << " , ID = " << cmd->getId();
            break;
        }
        case CommandType::ERR: {
            const auto* cmd = dynamic_cast<const ErrorCommand*>(command.get());
            ss << "Received command ERR: Type = " << cmd->getCode()
               << " , Msg = " << cmd->getMessage();
            break;
        }
        case CommandType::LOGIN_RESP: {
            const auto* cmd = dynamic_cast<const LoginResponseCommand*>(command.get());
            ss << "Received command LOGIN_RESP: Response = " << (cmd->getResponse() ? "True" : "False")
               << " , UUID = " << cmd->getUuid();
            break;
        }
        case CommandType::NOTIF_RESP_SERVER: {
            const auto* cmd = dynamic_cast<const NotificationResponseServerCommand*>(command.get());
            ss << "Received command NOTIF_RESP_SERVER: Response = " << (cmd->getResponse() ? "True" : "False")
               << " , UUID = " << cmd->getUuid();
            break;
        }
        case CommandType::VALIDATE_RESP_CLIENT: {
            const auto* cmd = dynamic_cast<const ValidateResponseClientCommand*>(command.get());
            ss << "Received command VALIDATE_RESP_CLIENT: Response = " << (cmd->getResp() ? "True" : "False")
               << " , UUID = " << cmd->getUuid();
            break;
        }
        default:
            // For commands that are valid, but the DC shouldn't necessarily receive or strictly log
            ss << "Invalid or Unexpected command type: " << static_cast<int>(command.get()->getType());
    }
    return ss.str();
}

int main() {
    int client_socket = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DS_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        return -1;
    }

    std::cout << "Connecting to Dummy Server on port " << DS_PORT << "...\n";
    if (connect(client_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    std::cout << "Connected! Type help for commands.\n";

    fd_set read_set;
    while (true) {
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        FD_SET(client_socket, &read_set);

        int activity = select(client_socket + 1, &read_set, NULL, NULL, NULL);

        if (activity < 0) break;

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
                std::cout << "\nServer disconnected.\n";
                break;
            } else {
                std::string data(buffer, valread);
                std::unique_ptr<Command> command;
                try {
                    command = CommandFactory::create(data);
                } catch (...) {
                    command = nullptr;
                }

                if (command) {
                    std::string reply = handleCommand(command);
                    std::cout << "[DC Log] " << reply << "\n";

                    send(client_socket, reply.c_str(), reply.length(), 0);
                } else {
                    std::cout << "[DS Reply] Invalid format or ID: " << data << "\n";
                }
            }
        }
    }

    close(client_socket);
    return 0;
}