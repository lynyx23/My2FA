#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cerrno>
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

#define PORT 27701 // AuthServer port
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE); // TO-DO c++ style errors
}

std::string handleCommand(const std::unique_ptr<Command>& command) {
    std::stringstream ss;
    switch(command.get()->getType()) {
        case CommandType::CONN: {
            const auto* cmd = dynamic_cast<const ConnectCommand*>(command.get());
            ss << "Received command CONN: Type = " << static_cast<int>(cmd->getType())
                <<" , ID = " << cmd->getId();
            break;
        }
        case CommandType::ERR: {
            const auto* cmd = dynamic_cast<const ErrorCommand*>(command.get());
            ss << "Received command ERR: Type = " << static_cast<int>(cmd->getCode())
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

int main(int argc, char *argv[]) {
    int server_socket, new_socket, activity, valread;
    int addrlen, sd;
    int client_socket[MAX_CLIENTS] = {0};
    int max_sd;

    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    fd_set master_set;
    fd_set read_set, write_set;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_exit("socket failed");
    }

    // int opt = 1;
    // if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
    //     error_exit("setsockopt");
    // }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error_exit("bind failed");
    }

    if (listen(server_socket, 3) < 0) {
        error_exit("listen");
    }

    std::cout << "Listening on port: " << PORT << "\n";

    FD_ZERO(&master_set);
    FD_SET(server_socket, &master_set);

    while (true) {
        read_set = master_set;
        write_set = master_set;

        activity = select(max_sd + 1, &read_set, &write_set, NULL, NULL);

        if ((activity < 0)) {
            std::cout << "Select error: " << "\n";
            continue;
        }

        if (FD_ISSET(server_socket, &read_set)) {
            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                error_exit("accept");
            }

            std::cout << "New connection. SD: " << new_socket << ", IP: " << inet_ntoa(address.sin_addr) << "\n";

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }

            FD_SET(new_socket, &master_set);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (sd > 0 && FD_ISSET(sd, &read_set)) {

                memset(buffer, 0, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE - 1);

                if (valread == 0) {
                    std::cout << "Host disconnected. IP: " << inet_ntoa(address.sin_addr) << "\n";

                    close(sd);
                    client_socket[i] = 0;
                    FD_CLR(client_socket[i], &master_set);
                } else {
                    std::cout << "Received from SD " << sd << ": " << buffer;
                    send(sd, buffer, valread, 0);
                }
            }
        }
    }

    return 0;
}