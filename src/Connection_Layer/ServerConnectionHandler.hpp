#ifndef MY2FA_SERVERCONNECTIONHANDLER_HPP
#define MY2FA_SERVERCONNECTIONHANDLER_HPP

#include <functional>
#include <memory>
#include <mutex>
#include "../Command_Layer/Base/Command.hpp"


class ServerConnectionHandler {
public:
    using CommandCallback = std::function<void(int client_fd, std::unique_ptr<Command>)>;
    using ConnectCallback = std::function<void(int client_fd)>;
    using DisconnectCallback = std::function<void(int client_fd)>;

    explicit ServerConnectionHandler(int port);
    ~ServerConnectionHandler() { m_disconnect(); }

    // makes sure a single object is created
    ServerConnectionHandler(const ServerConnectionHandler &) = delete;
    ServerConnectionHandler &operator=(const ServerConnectionHandler &) = delete;

    void setCommandCallback(const CommandCallback &callback);
    void setConnectCallback(const ConnectCallback &callback);
    void setDisconnectCallback(const DisconnectCallback &callback);

    [[nodiscard]] int getSocket() const;

    [[nodiscard]] bool isRunning() const;

    void update();

    // ReSharper disable once CppMemberFunctionMayBeStatic
    void sendCommand(int client_sd, const std::unique_ptr<Command> &cmd) const;
    void broadcastCommand(const std::unique_ptr<Command> &cmd) const;

private:
    int m_port;
    int m_socket;
    std::vector<int> m_client_sockets;
    mutable std::mutex m_mutex;

    CommandCallback m_commandCallback;
    ConnectCallback m_connectCallback;
    DisconnectCallback m_disconnectCallback;

    void m_setupSocket();
    void m_handleConnection();
    [[nodiscard]] bool m_handleData(int client_sd) const;
    void m_disconnect();
};

#endif //MY2FA_SERVERCONNECTIONHANDLER_HPP
