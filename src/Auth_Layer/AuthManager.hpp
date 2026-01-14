#ifndef MY2FA_LOGGER_HPP
#define MY2FA_LOGGER_HPP

#include <map>
#include <optional>
#include <string>

struct PendingPairing {
    std::string d_username;
    std::string app_id;
    std::string secret;
};

class AuthManager {
public:
    explicit AuthManager(const std::string &server_type);

    [[nodiscard]] bool loginUser(const std::string &username, const std::string &password);
    [[nodiscard]] bool registerUser(const std::string &username, const std::string &password);

    [[nodiscard]] std::string startPairing(const std::string &d_username, const std::string &app_id);
    [[nodiscard]] std::optional<std::pair<std::string, std::string>> finishPairing(const std::string &a_username, const std::string &token);

    void show();
private:
    const std::string m_server_type;
    std::map<std::string, PendingPairing> m_pending_pairings;

    [[nodiscard]] std::string m_hashPassword(const std::string& password, const std::string& salt) ;
    [[nodiscard]] std::string m_generateSalt();
    [[nodiscard]] std::string m_generateSecret();
    [[nodiscard]] std::string m_generateToken();
};

#endif //MY2FA_LOGGER_HPP