#ifndef MY2FA_SESSIONMANAGER_HPP
#define MY2FA_SESSIONMANAGER_HPP

#pragma once
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>

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

struct Session {
    int id;
    EntityType type;
    std::string uuid;
    std::string secret;
    std::atomic<bool> isLogged;
    std::atomic<bool> isValid;
};

class SessionManager {
public:
    SessionManager() = default;

    void addSession(int id);
    void removeSession(int id);

    void handleHandshake(int id, EntityType type);
    void displayConnections();
    [[nodiscard]] bool checkLoggedIn(const std::string &uuid) const;
    void logout(int id);

    void setIsLogged(int id, bool isLogged);
    void setUUID(int id, const std::string &uuid);
    void setSecret(int id, const std::string &secret);

    int getID(int id) const;
    [[nodiscard]] std::string getUUID(int id) const;
    [[nodiscard]] EntityType getEntityType(int id) const;
    bool getIsLogged(int id) const;
    [[nodiscard]] std::vector<std::shared_ptr<Session>> getAllSessions() const;

private:
    std::map<int, std::shared_ptr<Session>> m_sessions;
    mutable std::mutex m_mutex;
};

#endif // MY2FA_SESSIONMANAGER_HPP
