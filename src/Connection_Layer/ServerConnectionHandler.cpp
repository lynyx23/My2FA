#include "ServerConnectionHandler.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include "Command_Layer/CommandFactory.hpp"

ServerConnectionHandler::ServerConnectionHandler(const int port) : m_port(port), m_socket(-1) {
    try {
        m_setupSocket();
    } catch (std::exception &e) {
        std::cerr << "Socket Setup Failed: " << e.what() << "\n";
        exit(EXIT_FAILURE);
    }
}

void ServerConnectionHandler::setCommandCallback(const CommandCallback &callback) { m_commandCallback = callback; };

void ServerConnectionHandler::setConnectCallback(const ConnectCallback &callback) { m_connectCallback = callback; }

void ServerConnectionHandler::setDisconnectCallback(const DisconnectCallback &callback) {
    m_disconnectCallback = callback;
}

int ServerConnectionHandler::getSocket() const { return m_socket; }

bool ServerConnectionHandler::isRunning() const { return m_socket > 0; }

void ServerConnectionHandler::update() {
    if (m_socket <= 0)
        return;

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(m_socket, &read_set);

    int max_sd = m_socket;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (const int sd: m_client_sockets) {
            if (sd > 0)
                FD_SET(sd, &read_set);
            if (sd > max_sd)
                max_sd = sd;
        }
    }

    timeval timeout{0, 5000};
    if (select(max_sd + 1, &read_set, nullptr, nullptr, &timeout) < 0) {
        if (errno != EINTR) {
            std::cerr << "Select error : " << errno << ".\n Removing broken sockets ...\n";
            for (auto sd = m_client_sockets.begin(); sd != m_client_sockets.end();) {
                if (fcntl(*sd, F_GETFL) == -1) {
                    std::cerr << "Removing socket " << *sd << "\n";
                    sd = m_client_sockets.erase(sd);
                } else
                    ++sd;
            }
        }
        return;
    }

    if (FD_ISSET(m_socket, &read_set)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handleConnection();
    }

    std::vector<int> client_sockets_snapshot;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        client_sockets_snapshot = m_client_sockets;
    }

    for (auto sd: client_sockets_snapshot) {
        if (FD_ISSET(sd, &read_set)) {
            if (!m_handleData(sd)) {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (auto it = std::find(m_client_sockets.begin(), m_client_sockets.end(), sd);
                    it != m_client_sockets.end()) {
                    m_client_sockets.erase(it);
                }
            }
        }
    }
}

void ServerConnectionHandler::sendCommand(const int client_sd, const std::unique_ptr<Command> &cmd) const {
    const std::string data = cmd->serialize();
    std::lock_guard<std::mutex> lock(m_mutex);
    if (fcntl(client_sd, F_GETFD) != -1) {
        send(client_sd, data.c_str(), data.length(), 0);
    }
}

void ServerConnectionHandler::broadcastCommand(const std::unique_ptr<Command> &cmd) const {
    for (auto sd = m_client_sockets.begin(); sd != m_client_sockets.end(); ++sd) {
        sendCommand(*sd, cmd);
    }
}

void ServerConnectionHandler::m_setupSocket() {
    sockaddr_in address{};

    if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "[SCH Error] Socket failed!\n";
    }

    int opt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        std::cerr << "[SCH Error] setsockopt failed!\n";
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) < 0) {
        std::cerr << "[SCH Error] Bind failed!\n";
    }

    if (listen(m_socket, 3) < 0) {
        std::cerr << "listen";
    }

    std::cout << "Server started! Type help for commands.\n";
    std::cout << "Server listening on port: " << m_port << "\n";
}

void ServerConnectionHandler::m_handleConnection() {
    sockaddr_in client_address{};
    socklen_t addrlen = sizeof(client_address);

    const int new_socket = accept(m_socket, reinterpret_cast<struct sockaddr *>(&client_address), &addrlen);
    if (new_socket < 0) {
        std::cerr << "Accept client error\n";
        return;
    }

    m_client_sockets.push_back(new_socket);

    if (m_connectCallback) {
        m_connectCallback(new_socket);
    }
}

bool ServerConnectionHandler::m_handleData(const int client_sd) const {
    char buffer[1024] = {0};
    memset(buffer, 0, 1024);

    const int valread = read(client_sd, buffer, 1024);
    if (valread <= 0) {
        std::cout << "Client disconnected.\n";
        if (m_disconnectCallback) {
            m_disconnectCallback(client_sd);
        }
        close(client_sd);
        return false;
    }
    const std::string data(buffer, valread);
    std::unique_ptr<Command> command;
    try {
        command = CommandFactory::create(data);
    } catch (...) {
        command = nullptr;
    }
    if (m_commandCallback && command)
        m_commandCallback(client_sd, std::move(command));
    return true;
}

void ServerConnectionHandler::m_disconnect() {
    if (m_socket > 0) {
        close(m_socket);
        m_socket = -1;
        for (const int sd: m_client_sockets)
            close(sd);
        m_client_sockets.clear();
        std::cout << "Server stopped.\n";
    }
}
