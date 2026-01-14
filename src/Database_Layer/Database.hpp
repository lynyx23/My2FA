#ifndef MY2FA_DATABASE_HPP
#define MY2FA_DATABASE_HPP
#include <SQLiteCpp/Database.h>
#include <optional>

namespace Database {
    struct UserDTO {
        std::string username;
        std::string pass_hash;
        std::string salt;
#ifdef A_SERVER
        std::string totp_secret;
#endif
    };

    void init(const std::string &server_type);

    [[nodiscard]] bool createUser(const UserDTO &user);

    [[nodiscard]] std::optional<UserDTO> getUser(const std::string &username);
    [[nodiscard]] bool pairUser(const std::string &a_username,
        const std::string &d_username, const std::string &app_id, const std::string &secret);
    [[nodiscard]] std::optional<std::string> getSecret(const std::string &d_username, const std::string &app_id);

    void updateSecret(const std::string &username, const std::string &secret);
    void removeUser(const std::string &username);

    [[nodiscard]] std::optional<std::string> getD_username(const std::string &a_username, const std::string &app_id);
    [[nodiscard]] std::map<std::string, std::string> getSecretPairings(const std::string &username);
    void show();
}

#endif // MY2FA_DATABASE_HPP
