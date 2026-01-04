#include <sstream>
#include <sys/types.h>
#include <sys/time.h>
#include <csignal>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"

#define DS_PORT 27702      // Dummy Server Port
#define AS_PORT 27701      // Auth Server Port
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

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
        case CommandType::LOGIN_REQ: {
            const auto *cmd = dynamic_cast<const LoginRequestCommand *>(command.get());
            ss << "Received command LOGIN_REQ: User = " << cmd->getUsername()
                    << " , Password = " << cmd->getPassword();
            break;
        }
        case CommandType::REQ_NOTIF_CLIENT: {
            const auto *cmd = dynamic_cast<const RequestNotificationClientCommand *>(command.get());
            ss << "Received command REQ_NOTIF_CLIENT: UUID = " << cmd->getUuid();
            break;
        }
        case CommandType::VALIDATE_CODE_CLIENT: {
            const auto *cmd = dynamic_cast<const ValidateCodeClientCommand *>(command.get());
            ss << "Received command VALIDATE_CODE_CLIENT: Code = " << cmd->getCode()
                    << " , UUID = " << cmd->getUuid();
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
            ss << "Received Command ID: " << static_cast<int>(command->getType());
    }
    return ss.str();
}

void handleUserInput(ServerConnectionHandler &ds_handler,
                     ClientConnectionHandler *as_handler, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << "  clients                                     : List all active client file descriptors.\n"
                << "  conn <type> <id>                            : Send Handshake to AS\n"
                << "  reconnect                                   : Reconnect to AS\n"
                << "  err <code> <msg>                            : Send Error to AS\n"
                << "  req_notif_server <uuid> <appid>             : Send Notification Request to AS\n"
                << "  notif_resp_server <resp> <uuid>             : Send Notification Result to DC\n"
                << "  validate_code_server <code> <uuid> <appid>  : Send Code Verify Request to AS\n"
                << "  validate_resp_client <resp> <uuid>          : Send Code Result to DC\n"
                << "  exit                                        : Exit\n";
        return;
    } else if (args[0] == "reconnect") return;
    // } else if (args[0] == "clients") {
    //     std::cout << "[DS Log] Active Sockets: ";
    //     bool found = false;
    //     // for (int i = 0; i < MAX_CLIENTS; ++i) {
    //     //     if (client_socket[i] > 0) {
    //     //         std::cout << client_socket[i] << " ";
    //     //         found = true;
    //     //     }
    //     // }
    //     if (!found) std::cout << "None.";
        // std::cout << "\n";
        // return;
    else if (args[0] == "conn") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: conn <type> <id>\n";
            return;
        }
        command = std::make_unique<ConnectCommand>(args[1], args[2]);
    } else if (args[0] == "err") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: err <code> <msg>\n";
            return;
        }
        command = std::make_unique<ErrorCommand>(std::stoi(args[1]), args[2]);
    } else if (args[0] == "req_notif_server") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: req_notif_server <uuid> <appid>\n";
            return;
        }
        command = std::make_unique<RequestNotificationServerCommand>(args[1], std::stoi(args[2]));
    } else if (args[0] == "notif_resp_server") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: notif_resp_server <1/0> <uuid>\n";
            return;
        }
        bool resp = (args[1] == "1" || args[1] == "true");
        command = std::make_unique<NotificationResponseServerCommand>(resp, args[2]);
    } else if (args[0] == "validate_code_server") {
        if (args.size() < 4) {
            std::cout << "[DS Error] Usage: validate_code_server <code> <uuid> <appid>\n";
            return;
        }
        command = std::make_unique<ValidateCodeServerCommand>(std::stoi(args[1]), args[2], std::stoi(args[3]));
    } else if (args[0] == "validate_resp_client") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: validate_resp_client <1/0> <uuid>\n";
            return;
        }
        bool resp = (args[1] == "1" || args[1] == "true");
        command = std::make_unique<ValidateResponseClientCommand>(resp, args[2]);
    } else {
        std::cout << "[DS Error] Unknown command! Type help.\n";
        return;
    }

    if (command) {
        const std::string data = command->serialize();

        switch (command->getType()) {
            case CommandType::CONN:
            case CommandType::ERR:
            case CommandType::REQ_NOTIF_SERVER:
            case CommandType::VALIDATE_CODE_SERVER:
                // try {
                //     as_handler->sendCommand(command);
                //     std::cout << "[DS -> AS] Sent: " << data << "\n";
                // } catch (std::exception &e) {
                //     std::cerr << "[DS Error] Not connected to Auth Server.\n" << e.what() << "\n";
                // }
                if (as_handler && as_handler->isRunning()) {
                    as_handler->sendCommand(command);
                    std::cout << "[DS -> AS] Sent: " << data << "\n";
                } else {
                    std::cerr << "[DS Error] Not connected to Auth Server.\n";
                }
                break;

            case CommandType::NOTIF_RESP_SERVER:
            case CommandType::VALIDATE_RESP_CLIENT:
                std::cout << "[DS -> DC] Sent: " << data << "\n";
                // For now it sends this to every client
                ds_handler.broadcastCommand(command);
                break;

            default:
                std::cout << "[DS Error] No routing rule for Command ID "
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
    signal(SIGPIPE, SIG_IGN);

    ServerConnectionHandler ds_handler(DS_PORT);

    ds_handler.setCommandCallback([&](const int client_fd, const std::unique_ptr<Command> &command) {
        std::cout << "[DS Log] Handling command from SD " << client_fd
                << "\n" << handleCommand(command) << "\n";
    });
    ds_handler.setConnectCallback([&](const int client_fd) {
        std::cout << "[DS Log] New client connected: " << client_fd << "\n";
    });
    ds_handler.setDisconnectCallback([&](const int client_fd) {
        std::cout << "[DS Log] Client disconnected: " << client_fd << "\n";
    });

    bool as_connected;
    std::unique_ptr<ClientConnectionHandler> as_handler = nullptr;
    try {
        as_handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", AS_PORT);
        as_handler->setCallback([&](const std::unique_ptr<Command> &command) {
            std::cout << "[DS Log] Handling command from AS ...\n"
                    << handleCommand(command) << "\n";
        });
        as_connected = true;
    } catch (...) {
        as_connected = false;
        std::cerr << "[DS Error] Could not connect to AS.\n";
    }

    bool run = true;
    while (run) {
        ds_handler.update();
        if (as_connected && as_handler) {
            if (!as_handler->isRunning()) {
                std::cerr << "[DS Error] Auth Server disconnected.\n";
                as_connected = false;
            } else as_handler->update();
        }

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);

            if (split(input)[0] == "exit") run = false;
            if (split(input)[0] == "reconnect") {
                try {
                    as_handler = std::make_unique<ClientConnectionHandler>("127.0.0.1", AS_PORT);
                    as_handler->setCallback([&](const std::unique_ptr<Command> &command) {
                        std::cout << "[DS Log] Handling command from AS ...\n"
                                << handleCommand(command) << "\n";
                    });
                    as_connected = true;
                } catch (...) {
                    as_connected = false;
                    std::cerr << "[DS Error] Could not reconnect to Auth Server.\n";
                }
            }
            if (!input.empty()) {
                handleUserInput(ds_handler, as_handler.get(), input);
            }
        }
    }
}
