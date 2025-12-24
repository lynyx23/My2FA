#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "../Connection_Layer/ClientConnectionHandler.hpp"

#define DS_PORT 27702

void handleUserInput(ClientConnectionHandler &handler, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << "  help                      : Show this menu\n"
                << "  conn <id>                 : Handshake (e.g. conn;dc_01)\n"
                << "  login <user> <pass>       : Credential Login (e.g. login;user;pass)\n"
                << "  req_notif <uuid>          : Request Notification Login (e.g. req_notif;uuid_1234)\n"
                << "  code <code> <uuid>        : Enter Code (e.g. code;123456;uuid_1234)\n"
                << "  exit                      : Quit\n";
        return;
    } else if (args[0] == "exit") {
        exit(0);
    } else if (args[0] == "conn") {
        if (args.size() < 2) {
            std::cout << "[DC Error] Usage: conn <id>\n";
            return;
        }
        command = std::make_unique<ConnectCommand>("DUMMY_CLIENT", args[1]);
    } else if (args[0] == "login") {
        if (args.size() < 3) {
            std::cout << "[DC Error] Usage: login <user> <pass>\n";
            return;
        }
        command = std::make_unique<LoginRequestCommand>(args[1], args[2]);
    } else if (args[0] == "req_notif") {
        if (args.size() < 2) {
            std::cout << "[DC Error] Usage: req_notif <uuid>\n";
            return;
        }
        command = std::make_unique<RequestNotificationClientCommand>(args[1]);
    } else if (args[0] == "code") {
        if (args.size() < 3) {
            std::cout << "[DC Error] Usage: code <code> <uuid>\n";
            return;
        }
        try {
            command = std::make_unique<ValidateCodeClientCommand>(std::stoi(args[1]), args[2]);
        } catch (...) {
            std::cout << "[DC Error] Code must be an integer.\n";
            return;
        }
    } else {
        std::cout << "[DC Error] Unknown command! Type help.\n";
        return;
    }

    if (command) {
        std::string data = command->serialize();
        handler.sendCommand(command);
        std::cout << "[DC Sent] " << data << "\n> " << std::flush;
    }
}

std::string handleCommand(const std::unique_ptr<Command> &command) {
    std::ostringstream ss;
    switch (command->getType()) {
        case CommandType::CONN: {
            const auto *cmd = dynamic_cast<const ConnectCommand *>(command.get());
            ss << "Received command CONN: Type = " << cmd->getConnectionType()
                    << " , ID = " << cmd->getId();
            break;
        }
        case CommandType::ERR: {
            const auto *cmd = dynamic_cast<const ErrorCommand *>(command.get());
            ss << "Received command ERR: Type = " << cmd->getCode()
                    << " , Msg = " << cmd->getMessage();
            break;
        }
        case CommandType::LOGIN_RESP: {
            const auto *cmd = dynamic_cast<const LoginResponseCommand *>(command.get());
            ss << "Received command LOGIN_RESP: Response = " << (cmd->getResponse() ? "True" : "False")
                    << " , UUID = " << cmd->getUuid();
            break;
        }
        case CommandType::NOTIF_RESP_SERVER: {
            const auto *cmd = dynamic_cast<const NotificationResponseServerCommand *>(command.get());
            ss << "Received command NOTIF_RESP_SERVER: Response = " << (cmd->getResponse() ? "True" : "False")
                    << " , UUID = " << cmd->getUuid();
            break;
        }
        case CommandType::VALIDATE_RESP_CLIENT: {
            const auto *cmd = dynamic_cast<const ValidateResponseClientCommand *>(command.get());
            ss << "Received command VALIDATE_RESP_CLIENT: Response = " << (cmd->getResp() ? "True" : "False")
                    << " , UUID = " << cmd->getUuid();
            break;
        }
        default:
            ss << "Invalid or Unexpected command type: " << static_cast<int>(command->getType());
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
    ClientConnectionHandler handler("127.0.0.1", DS_PORT);
    handler.setCallback([&](const std::unique_ptr<Command> &command) {
        std::cout << "[DC Log] Handling command ...\n" << handleCommand(command) << "\n";
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
