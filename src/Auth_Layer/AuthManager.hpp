#ifndef MY2FA_LOGGER_HPP
#define MY2FA_LOGGER_HPP

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>

class AuthManager {
public:
    explicit AuthManager(const std::string& server_type);
    [[nodiscard]] bool loginUser(const std::string& username, const std::string& password) const;
    void testAddUser(const std::string& username, const std::string& password);
    void testRemoveUser(const std::string& username);
    void show() const;
    void init();
private:
    SQLite::Database m_db;
    [[nodiscard]] static std::string m_generateUUID(const std::string& username) ;
    [[nodiscard]] static std::string m_hashPassword(const std::string& password, const std::string& salt) ;
    [[nodiscard]] static std::string m_generateSalt() ;
};

#endif //MY2FA_LOGGER_HPP