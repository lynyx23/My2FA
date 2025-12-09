#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>

#include "../Command_Layer/CommandFactory.hpp"
#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "../Command_Layer/Code_Login/CodeLoginCommands.hpp"

#define PORT 27701 // Auth port
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

int client_socket[MAX_CLIENTS] = {0};

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE); // TO-DO c++ style errors
}

std::string handleCommand(const std::unique_ptr<Command>& command) {
    std::ostringstream ss;
    switch(command.get()->getType()) {
        case CommandType::CONN: {
            const auto* cmd = dynamic_cast<const ConnectCommand*>(command.get());
            ss << "Received command CONN: Type = " << cmd->getConnectionType()
                <<" , ID = " << cmd->getId();
            break;
        }
        case CommandType::ERR: {
            const auto* cmd = dynamic_cast<const ErrorCommand*>(command.get());
            ss << "Received command ERR: Type = " << cmd->getCode()
                << " , Msg = " << cmd->getMessage();
            break;
        }
        case CommandType::LOGIN_REQ: {
            const auto* cmd = dynamic_cast<const LoginRequestCommand*>(command.get());
            ss << "Received command LOGIN_REQ: User = " << cmd->getUsername()
                << " , Password = " << cmd->getPassword();
            break;
        }
        case CommandType::REQ_NOTIF_SERVER: {
            const auto* cmd = dynamic_cast<const RequestNotificationServerCommand*>(command.get());
            ss << "Received command REQ_NOTIF_SERVER: UUID = " << cmd->getUuid()
                << " , AppID = " << cmd->getAppid();
            break;
        }
        case CommandType::NOTIF_RESP_CLIENT: {
            const auto* cmd = dynamic_cast<const NotificationResponseClientCommand*>(command.get());
            ss << "Received command NOTIF_RESP_CLIENT: Response = "
                << (cmd->getResponse() ? "True" : "False")
                << " , AppID = " << cmd->getAppid();
            break;
        }
        case CommandType::REQ_CODE_CLIENT: {
            const auto* cmd = dynamic_cast<const RequestCodeClientCommand*>(command.get());
            ss << "Received command REQ_CODE_CLIENT: UUID = " << cmd->getUuid()
                << " , AppID = " << cmd->getAppid();
            break;
        }
        case CommandType::VALIDATE_CODE_SERVER: {
            const auto* cmd = dynamic_cast<const ValidateCodeServerCommand*>(command.get());
            ss << "Received command VALIDATE_CODE_SERVER: Code = " << cmd->getCode()
                << " , UUID = " << cmd->getUuid()
                << " , AppID = " << cmd->getAppid();
            break;
        }
        default:
            // For commands that are valid, but the AS shouldn't receive
            ss << "Invalid command type: " << static_cast<int>(command.get()->getType());
    }
    return ss.str();
}

void handleUserInput(int server_socket, std::string input) {
    auto args = split(input); // Assuming you add the splitInput helper function
    if (args.empty()) return;

    if (args[0] == "help") {
        std::cout << "  clients          : List all active client file descriptors.\n"
                  << "  send_notif <sd>  : Sends a test notification to socket <sd>.\n"
                  << "  exit             : Shut down the server.\n"
        ;
    }
    else if (args[0] == "clients") {
        std::cout << "[AS Log] Active Sockets: ";
        bool found = false;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (client_socket[i] > 0) {
                std::cout << client_socket[i] << " ";
                found = true;
            }
        }
        if (!found) std::cout << "None.";
        std::cout << "\n";
    }
    else if (args[0] == "send_notif") {
        if (args.size() != 2) {
            std::cout << "[AS Error] Usage: send_notif <socket_descriptor_id>\n";
            return;
        }
        try {
            int target_sd = std::stoi(args[1]);

            // Generate a sample notification command (23;APPID)
            SendNotificationCommand notif(99); // AppID 99
            std::string data = notif.serialize();

            // Send the command
            if (send(target_sd, data.c_str(), data.length(), 0) > 0) {
                std::cout << "[AS Send] Sent test notification (23) to SD " << target_sd << "\n";
            } else {
                std::cerr << "[AS Error] Failed to send to socket " << target_sd << "\n";
            }
        } catch (...) {
            std::cerr << "[AS Error] Invalid socket ID provided.\n";
        }
    }
    else if (args[0] == "exit") {
        std::cout << "[AS Log] Shutting down...\n";
        close(server_socket);
        exit(0);
    }
    else {
        std::cout << "[AS Error] Unknown command. Type 'help'.\n";
    }
}

int main(int argc, char *argv[]) {
    int server_socket, new_socket;
    int addrlen, sd;
    int max_sd;

    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    fd_set master_set;
    fd_set read_set;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_exit("socket failed");
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        error_exit("setsockopt");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error_exit("bind failed");
    }

    if (listen(server_socket, 3) < 0) {
        error_exit("listen");
    }

    std::cout << "[AS Log] Auth Server started! Type help for commands.\n";
    std::cout << "[AS Log] Auth Server listening on port: " << PORT << "\n";

    FD_ZERO(&master_set);
    FD_SET(server_socket, &master_set);
    max_sd = server_socket;

    while (true) {
        read_set = master_set;
        FD_SET(STDIN_FILENO, &read_set);

        // Recalculating max_sd for every loop iteration to avoid client disconnect issues
        max_sd = server_socket;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (sd > 0 && sd > max_sd) {
                max_sd = sd;
            }
        }
        if (STDIN_FILENO > max_sd) max_sd = STDIN_FILENO;

        int activity = select(max_sd + 1, &read_set, NULL, NULL, NULL);

        if ((activity < 0)) {
            std::cout << "Select error: " << "\n";
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &read_set)) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                handleUserInput(server_socket, input);
            }
            continue;
        }

        if (FD_ISSET(server_socket, &read_set)) {
            addrlen = sizeof(address);
            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                error_exit("accept");
            }

            std::cout << "[AS Log] New connection. SD: " << new_socket << ", IP: " << inet_ntoa(address.sin_addr) << "\n";

            FD_SET(new_socket, &master_set);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (sd > 0 && FD_ISSET(sd, &read_set)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE - 1);

                if (valread == 0) {
                    // Getting the address of this connection
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    std::cout << "[AS Log] Host disconnected. IP: " << inet_ntoa(address.sin_addr) << "\n";

                    close(sd);
                    FD_CLR(client_socket[i], &master_set);
                    client_socket[i] = 0;
                } else {
                    // Using constructor because the buffer might not be null terminated if full
                    std::string data(buffer, valread);
                    std::unique_ptr<Command> command = CommandFactory::create(data);
                    std::string response;

                    if(command) {
                        response = handleCommand(command);
                    }
                    else {
                        response = "Error : Invalid format or ID : " + data;
                    }
                    std::cout << "[AS Log] SD : " << sd << " , Response : " << response << "\n";
                    send(sd, response.c_str(), response.length(), 0);
                }
            }
        }
    }
    return 0;
}