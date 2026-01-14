#ifndef MY2FA_SESSIONMANAGER_HPP
#define MY2FA_SESSIONMANAGER_HPP

#pragma once
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "Command_Layer/Base/EntityType.hpp"

inline std::string stringifyEntityType(const EntityType &type) {
    switch (type) {
        case EntityType::AUTH_SERVER:
            return "AUTH_SERVER";
        case EntityType::AUTH_CLIENT:
            return "AUTH_CLIENT";
        case EntityType::DUMMY_SERVER:
            return "DUMMY_SERVER";
        case EntityType::DUMMY_CLIENT:
            return "DUMMY_CLIENT";
        default:
            return "NOT_ASSIGNED";
    }
}

struct AC_Data {
    std::atomic<bool> isLogged = false;
    std::atomic<bool> isInCodeState = false;
    std::map<std::string, std::string> secret_pairs;
};

struct Session {
    int id;
    EntityType type;
    std::string identity; // username for clients and app_id for DS
    std::atomic<bool> isValid;
    std::optional<AC_Data> ac_data;
};

class SessionManager {
public:
    SessionManager() = default;

    void addSession(int id);
    void removeSession(int id);

    void handleHandshake(int id, EntityType type, const std::string &app_id);
    void displayConnections();
    void logout(int id);

    void setIsLogged(int id, bool isLogged);
    void setSecret(int id, const std::string &secret);
    void setSecretPairings(int id, const std::map<std::string, std::string> &pairings);
    void setIsInCodeState(int id, bool isInCodeState);
    void setIdentity(int id, const std::string &identity);

    int getIDFromUsername(const std::string &username) const;
    [[nodiscard]] std::shared_ptr<Session> getSession(int id) const;
    [[nodiscard]] std::string getSecret(int id) const;
    [[nodiscard]] EntityType getEntityType(int id) const;
    bool getIsLogged(int id) const;
    [[nodiscard]] std::string getIdentity(int id) const;
    [[nodiscard]] std::vector<std::shared_ptr<Session>> getActiveSessions() const;

private:
    std::map<int, std::shared_ptr<Session>> m_sessions;
    mutable std::mutex m_mutex;
};

#endif // MY2FA_SESSIONMANAGER_HPP
