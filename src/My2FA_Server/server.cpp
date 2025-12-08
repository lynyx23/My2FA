#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define PORT 9000
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
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