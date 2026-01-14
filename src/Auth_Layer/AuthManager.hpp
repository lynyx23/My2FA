#ifndef MY2FA_LOGGER_HPP
#define MY2FA_LOGGER_HPP

#include <map>
#include <optional>
#include <string>
#include "Session_Manager/SessionManager.hpp"

struct PendingPairing {
    std::string d_username;
    std::string app_id;
    std::string secret;
};

struct PendingNotification {
    std::string d_username;
    std::string app_id;
    int ds_fd;
};

class AuthManager {
public:
    explicit AuthManager(const std::string &server_type);

    [[nodiscard]] bool loginUser(const std::string &username, const std::string &password);
    [[nodiscard]] bool registerUser(const std::string &username, const std::string &password);

    [[nodiscard]] std::string startPairing(const std::string &d_username, const std::string &app_id);
    [[nodiscard]] std::optional<std::pair<std::string, std::string>> finishPairing(
        const std::string &a_username, const std::string &token);

    [[nodiscard]] int startNotification(const std::string &username, const std::string &app_id, std::string &reqID,
                          const int &ds_fd, SessionManager &session_manager);
    [[nodiscard]] int finishNotification(const std::string &reqID, std::string &d_username);


    void show();
private:
    const std::string m_server_type;
    std::map<std::string, PendingPairing> m_pending_pairings;
    std::map<std::string, PendingNotification> m_pending_notifications;

    [[nodiscard]] std::string m_hashPassword(const std::string& password, const std::string& salt) ;
    [[nodiscard]] std::string m_generateSalt();
    [[nodiscard]] std::string m_generateSecret();
    [[nodiscard]] std::string m_generateToken();
    [[nodiscard]] std::string m_generateReqID() const;
};

#endif //MY2FA_LOGGER_HPP