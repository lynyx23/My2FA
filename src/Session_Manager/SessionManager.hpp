#ifndef MY2FA_SESSIONMANAGER_HPP
#define MY2FA_SESSIONMANAGER_HPP

#pragma once
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "Command_Layer/Base/EntityType.hpp"

inline std::string stringifyEntityType(const EntityType &type) {
    switch (type) {
        case EntityType::AUTH_SERVER: return "AUTH_SERVER";
        case EntityType::AUTH_CLIENT: return "AUTH_CLIENT";
        case EntityType::DUMMY_SERVER: return "DUMMY_SERVER";
        case EntityType::DUMMY_CLIENT: return "DUMMY_CLIENT";
        default: return "NOT_ASSIGNED";
    }
}

struct Session {
    int id;
    EntityType type;
    std::string uuid;
    bool isLogged;
    bool isValid;
};

class SessionManager {
public:
    SessionManager() = default;

    void addSession(const int id) {
        m_sessions[id] = std::make_shared<Session>(
            Session{id, EntityType::NOT_ASSIGNED, "", false, true});
        std::cout << "[SM Log] Session added: " << id << "\n";
    }

    void removeSession(const int id) {
        if (const auto it = m_sessions.find(id); it != m_sessions.end()) {
            it->second->isValid = false;
            it->second->id=-1;
            m_sessions.erase(it);
            std::cout << "[SM Log] Session removed: " << id << "\n";
        }

        std::cout << "[SM Log] Session removed: " << id << "\n";
    }

    void m_handleHandshake(const int id, const EntityType type) {
        if (m_sessions.contains(id)) {
            if (m_sessions[id]->type == EntityType::NOT_ASSIGNED) {
                m_sessions[id]->type = type;
                std::cout << "[SM Log] Handshake successful: " << id << " " << type << "\n";
            }
        }
    }

    void displayConnections() {
        std::cout << "[SM Log] Active sessions:\n";
        for (auto &[id, session]: m_sessions) {
            std::cout << "ID: " << id << " | Type: " << stringifyEntityType(session->type)
                << " | UUID: " << session->uuid << " | isLogged: " << session->isLogged << "\n";
        }
    }

    void setIsLogged(const int id, const bool isLogged) {
        if (m_sessions.contains(id)) {
            m_sessions[id]->isLogged = isLogged;
        }
    }

    int getID(const int id) {
        return m_sessions[id]->id;
    }

    std::string getUUID(const int id) {
        return m_sessions[id]->uuid;
    }

    EntityType getEntityType(const int id) {
        return m_sessions[id]->type;
    }

    bool getIsLogged(const int id) {
        return m_sessions[id]->isLogged;
    }

private:
    std::map<int, std::shared_ptr<Session>> m_sessions;
};

#endif //MY2FA_SESSIONMANAGER_HPP