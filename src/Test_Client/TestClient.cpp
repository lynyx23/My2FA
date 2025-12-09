#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../Command_Layer/System_Commands/SystemCommands.hpp"
#include "../Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "../Command_Layer/Notification_Login/NotificationLoginCommands.hpp"

#define PORT 27701

void sendAndLog(int sock, const Command& cmd) {
    // 1. Serialize using your logic
    std::string data = cmd.serialize();
    std::cout << "\n[Client] Sending: " << data << std::endl;

    // 2. Send
    send(sock, data.c_str(), data.length(), 0);

    // 3. Wait for Reply (Blocking)
    char buffer[1024] = {0};
    int valread = read(sock, buffer, 1024);
    
    if (valread > 0) {
        std::cout << "[Server] Replied: " << std::string(buffer, valread) << std::endl;
    } else {
        std::cout << "[Client] Error: Server closed connection or failed to reply." << std::endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        return -1;
    }

    std::cout << "[Client] Connecting to Auth Server on port " << PORT << "..." << std::endl;
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed. Is the Server running?" << std::endl;
        return -1;
    }

    // --- TEST 1: Handshake ---
    ConnectCommand conn("AUTH_CLIENT", "phone_test_01");
    sendAndLog(sock, conn);

    // --- TEST 2: Login Request ---
    // Note: AS might just log this, usually it goes to DS, but AS can parse it
    LoginRequestCommand login("alice", "password123");
    sendAndLog(sock, login);

    // --- TEST 3: Notification Request ---
    RequestNotificationServerCommand notifReq("uuid-12345", 241);
    sendAndLog(sock, notifReq);

    // --- TEST 4: Invalid/Error Command ---
    // Sending a manually constructed error to see how server handles it
    ErrorCommand err(404, "Test Error Message");
    sendAndLog(sock, err);

    close(sock);
    return 0;
}