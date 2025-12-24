#ifndef MY2FA_CLIENTCONNECTIONHANDLER_HPP
#define MY2FA_CLIENTCONNECTIONHANDLER_HPP

#include <functional>
#include <memory>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../Command_Layer/Base/Command.hpp"
#include "../Command_Layer/CommandFactory.hpp"

class ClientConnectionHandler {
public:
    using CommandCallback = std::function<void(std::unique_ptr<Command>)>;

    ClientConnectionHandler(const std::string &ip, int port)
        : m_socket(0), m_port(port), m_ip(ip) {
        m_setupSocket();
    }

    // sanity check for wrong synthax
    // makes sure a single object is created
    ClientConnectionHandler(const ClientConnectionHandler &) = delete;

    ClientConnectionHandler &operator=(const ClientConnectionHandler &) = delete;

    void update() {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(m_socket, &read_set);

        timeval timeout{0, 5000};
        if (select(m_socket + 1, &read_set, nullptr, nullptr, &timeout) < 0) {
            std::cerr << "Select error\n";
            disconnect();
        }

        if (FD_ISSET(m_socket, &read_set)) m_handleData();
    }

    void sendCommand(const std::unique_ptr<Command> &cmd) const {
        std::string data = cmd->serialize();
        send(m_socket, data.c_str(), data.length(), 0);
    }

    void disconnect() {
        if (m_socket > 0) {
            close(m_socket);
            m_socket = 0;
            std::cerr << "Force disconnected from server.\n";
        }
    }

    void setCallback(const CommandCallback &callback) {
        m_callback = callback;
    };

    int getSocket() const {
        return m_socket;
    }

    ~ClientConnectionHandler() {
        close(m_socket);
    }

private:
    void m_setupSocket() {
        struct sockaddr_in serv_addr{};

        if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Socket creation error";
            disconnect();
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_port);

        if (inet_pton(AF_INET, m_ip.c_str(), &serv_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address";
            disconnect();
        }

        std::cout << "Connecting to Server on port " << m_port << "...\n";
        if (connect(m_socket, reinterpret_cast<struct sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
            disconnect();
            throw std::runtime_error("Failed to connect to " + m_ip + ":" + std::to_string(m_port));
        }

        std::cout << "Connected to server!\n";
    }

    void m_handleData() {
        char buffer[1024] = {0};

        memset(buffer, 0, 1024);
        if (const int valread = read(m_socket, buffer, 1024); valread == 0) {
            std::cerr << "\nReceived Server disconnected.\n";
            disconnect();
        } else {
            const std::string data(buffer, valread);
            std::unique_ptr<Command> command;
            try {
                command = CommandFactory::create(data);
            } catch (...) {
                command = nullptr;
            }
            if (m_callback && command) m_callback(std::move(command));
        }
    }

    int m_socket;
    int m_port;
    std::string m_ip;
    CommandCallback m_callback;
};

#endif //MY2FA_CLIENTCONNECTIONHANDLER_HPP
