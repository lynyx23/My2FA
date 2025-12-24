#ifndef MY2FA_SERVERCONNECTIONHANDLER_HPP
#define MY2FA_SERVERCONNECTIONHANDLER_HPP

#include <functional>
#include <memory>
#include <iostream>
#include <string>
#include <cstring>
#include <math.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../Command_Layer/Base/Command.hpp"
#include "../Command_Layer/CommandFactory.hpp"

class ServerConnectionHandler {
public:
    using CommandCallback = std::function<void(int client_fd, std::unique_ptr<Command>)>;
    using ConnectCallback = std::function<void(int client_fd)>;
    using DisconnectCallback = std::function<void(int client_fd)>;

    explicit ServerConnectionHandler(const int port) : m_port(port), m_socket(0) {
        try {
            m_setupSocket();
        } catch (std::exception e) {
            std::cerr << "Socket Setup Failed: " << e.what() << "\n";
            exit(EXIT_FAILURE);
        }
    }

    // makes sure a single object is created
    ServerConnectionHandler(const ServerConnectionHandler &) = delete;
    ServerConnectionHandler &operator=(const ServerConnectionHandler &) = delete;

    ~ServerConnectionHandler() {
        m_disconnect();
    }

    void setCommandCallback(const CommandCallback &callback) {
        m_commandCallback = callback;
    };

    void setConnectCallback(const ConnectCallback &callback) {
        m_connectCallback = callback;
    }

    void setDisconnectCallback(const DisconnectCallback &callback) {
        m_disconnectCallback = callback;
    }

    int getSocket() const {
        return m_socket;
    }

    void update() {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(m_socket, &read_set);

        int max_sd = m_socket;

        for (const int sd: m_client_sockets) {
            if (sd > 0)
                FD_SET(sd, &read_set);
            if (sd > max_sd) max_sd = sd;
        }

        timeval timeout{0, 5000};
        if (select(max_sd + 1, &read_set, nullptr, nullptr, &timeout) < 0) {
            std::cerr << "Select error\n";
        }

        if (FD_ISSET(m_socket, &read_set)) {
            m_handleConnection();
        }
        for (auto const sd = m_client_sockets.begin(); sd != m_client_sockets.end();) {
            if (FD_ISSET(*sd, &read_set)) {
                m_handleData(*sd);
            }
        }
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void sendCommand(const int client_sd, const std::unique_ptr<Command> &cmd) const {
        const std::string data = cmd->serialize();
        send(client_sd, data.c_str(), data.length(), 0);
    }

    void broadcastCommand(const std::unique_ptr<Command> &cmd) const {
        for (auto const sd = m_client_sockets.begin(); sd != m_client_sockets.end();) {
            sendCommand(*sd, cmd);
        }
    }

private:
    int m_port;
    int m_socket;
    std::vector<int> m_client_sockets;

    CommandCallback m_commandCallback;
    ConnectCallback m_connectCallback;
    DisconnectCallback m_disconnectCallback;

    void m_setupSocket() {
        sockaddr_in address;

        if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            std::cerr << "socket failed";
        }

        int opt = 1;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
            std::cerr << "setsockopt";
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(m_port);

        if (bind(m_socket, reinterpret_cast<struct sockaddr *>(&address),
                 sizeof(address)) < 0) {
            std::cerr << "bind failed";
        }

        if (listen(m_socket, 3) < 0) {
            std::cerr << "listen";
        }

        std::cout << "Server started! Type help for commands.\n";
        std::cout << "Server listening on port: " << m_port << "\n";
    }

    void m_handleConnection() {
        struct sockaddr_in client_address;
        socklen_t addrlen = sizeof(client_address);

        const int new_socket = accept(m_socket,
                                      reinterpret_cast<struct sockaddr *>(&client_address), &addrlen);
        if (new_socket < 0) {
            std::cerr << "Accept client error\n";
            return; // ide says it's redundant idk
        }

        m_client_sockets.push_back(new_socket);

        if (m_connectCallback) {
            m_connectCallback(new_socket);
        }
    }

    void m_handleData(const int client_sd) {
        char buffer[1024] = {0};
        memset(buffer, 0, 1024);

        const int valread = read(client_sd, buffer, 1024);
        if (valread == 0) {
            std::cout << "Client disconnected.\n";
            std::erase(m_client_sockets, client_sd);

            if (m_disconnectCallback) {
                m_disconnectCallback(client_sd);
            }

            close(client_sd);
        } else if (valread < 0) {
            std::cerr << "Read error\n";
            return;
        } else {
            const std::string data(buffer, valread);
            std::unique_ptr<Command> command;
            try {
                command = CommandFactory::create(data);
            } catch (...) {
                command = nullptr;
            }
            if (m_commandCallback && command) m_commandCallback(client_sd, std::move(command));
        }
    }

    void m_disconnect() {
        close(m_socket);
        for (const int sd: m_client_sockets) close(sd);
        m_client_sockets.clear();
        std::cout << "Server stopped.\n";
    }
};

#endif //MY2FA_SERVERCONNECTIONHANDLER_HPP
