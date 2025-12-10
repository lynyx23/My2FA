#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"

#define DS_PORT 27702      // Dummy Server Port
#define AS_PORT 27701      // Auth Server Port
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

int auth_server_socket = 0;
int client_socket[MAX_CLIENTS] = {0};

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void connectToAuthServer() {
    if (auth_server_socket > 0) return;

    struct sockaddr_in serv_addr;
    if ((auth_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "[DS Error] AS Socket creation error\n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(AS_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "[DS Error] Invalid AS address\n";
        return;
    }

    std::cout << "[DS Log] Connecting to Auth Server on " << AS_PORT << "...\n";
    if (connect(auth_server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[DS Error] Connection to AS Failed\n";
        close(auth_server_socket);
        auth_server_socket = 0;
        return;
    }

    // Send Handshake
    ConnectCommand conn("DUMMY_SERVER", "ds_01");
    std::string data = conn.serialize();
    send(auth_server_socket, data.c_str(), data.length(), 0);

    std::cout << "[DS Log] Connected to AS and sent handshake.\n";
}

std::string handleCommand(const std::unique_ptr<Command> &command) {
    std::ostringstream ss;
    switch (command.get()->getType()) {
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
            ss << "Received Command ID: " << static_cast<int>(command.get()->getType());
    }
    return ss.str();
}

void handleUserInput(int server_socket, const std::string &input, fd_set &master_set) {
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
    } else if (args[0] == "exit") {
        std::cout << "[DS Log] Shutting down...\n";
        close(server_socket);
        if (auth_server_socket > 0) close(auth_server_socket);
        exit(0);
    } else if (args[0] == "clients") {
        std::cout << "[DS Log] Active Sockets: ";
        bool found = false;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_socket[i] > 0) {
                std::cout << client_socket[i] << " ";
                found = true;
            }
        }
        if (!found) std::cout << "None.";
        std::cout << "\n";
        return;
    } else if (args[0] == "reconnect") {
        connectToAuthServer();
        if (auth_server_socket > 0) {
            FD_SET(auth_server_socket, &master_set);
        }
        return;
    } else if (args[0] == "conn") {
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
        std::string data = command->serialize();

        switch (command->getType()) {
            case CommandType::CONN:
            case CommandType::ERR:
            case CommandType::REQ_NOTIF_SERVER:
            case CommandType::VALIDATE_CODE_SERVER:
                if (auth_server_socket > 0) {
                    send(auth_server_socket, data.c_str(), data.length(), 0);
                    std::cout << "[DS -> AS] Sent: " << data << "\n";
                } else {
                    std::cout << "[DS Error] Not connected to Auth Server.\n";
                }
                break;

            case CommandType::NOTIF_RESP_SERVER:
            case CommandType::VALIDATE_RESP_CLIENT:
                std::cout << "[DS -> DC] Sent: " << data << "\n";
                // For now it sends this to every client
                for (int i = 0; i < MAX_CLIENTS; ++i) {
                    if (client_socket[i] > 0) {
                        send(client_socket[i], data.c_str(), data.length(), 0);
                    }
                }
                break;

            default:
                std::cout << "[DS Error] No routing rule for Command ID "
                        << static_cast<int>(command->getType()) << "\n";
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    int server_socket, new_socket, addrlen, max_sd, sd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    fd_set master_set, read_set;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_exit("socket failed");
    }
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DS_PORT);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        error_exit("bind failed");
    }
    if (listen(server_socket, 3) < 0) error_exit("listen");

    std::cout << "[DS Log] Dummy Server listening on port: " << DS_PORT << "\n";

    connectToAuthServer();

    FD_ZERO(&master_set);
    FD_SET(server_socket, &master_set);
    FD_SET(STDIN_FILENO, &master_set);

    if (auth_server_socket > 0) {
        FD_SET(auth_server_socket, &master_set);
    }

    while (true) {
        read_set = master_set;
        max_sd = server_socket;
        if (auth_server_socket > max_sd) max_sd = auth_server_socket;
        if (STDIN_FILENO > max_sd) max_sd = STDIN_FILENO;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_socket[i] > 0 && client_socket[i] > max_sd) {
                max_sd = client_socket[i];
            }
        }

        int activity = select(max_sd + 1, &read_set, NULL, NULL, NULL);
        if ((activity < 0)) std::cout << "[DS Error] Select error\n";

        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) handleUserInput(server_socket, input, master_set);
        }

        if (FD_ISSET(server_socket, &read_set)) {
            addrlen = sizeof(address);
            if ((new_socket = accept(server_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
                perror("accept");
            } else {
                std::cout << "[DS Log] New Client connection. SD: " << new_socket << "\n";
                FD_SET(new_socket, &master_set);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_socket[i] == 0) {
                        client_socket[i] = new_socket;
                        break;
                    }
                }
            }
        }

        // Handle AS
        if (auth_server_socket > 0 && FD_ISSET(auth_server_socket, &read_set)) {
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(auth_server_socket, buffer, BUFFER_SIZE - 1);

            if (valread <= 0) {
                std::cout << "[DS Log] Auth Server disconnected.\n";
                close(auth_server_socket);
                FD_CLR(auth_server_socket, &master_set);
                auth_server_socket = 0;
            } else {
                // AS sends plain text replies now
                std::string msg(buffer, valread);
                std::cout << "[DS Log] Received from AS: " << msg << "\n";
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0 && FD_ISSET(sd, &read_set)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE - 1);

                if (valread == 0) {
                    addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
                    std::cout << "[DS Log] Client disconnected.\n";
                    close(sd);
                    FD_CLR(sd, &master_set);
                    client_socket[i] = 0;
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
                        std::cout << "[DS Log] " << reply << "\n";

                        send(sd, reply.c_str(), reply.length(), 0);
                    } else {
                        std::cout << "[DS Reply] SD: " << sd << " " << data << "\n";
                    }
                }
            }
        }
    }
    return 0;
}
