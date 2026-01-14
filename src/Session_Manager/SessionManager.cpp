#include "SessionManager.hpp"
#include <SQLiteCpp/Database.h>
#include <iostream>
#include <memory>
#include <ranges>

void SessionManager::addSession(const int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // make_shared doesn't work because of the atomic bools which are un-copyable
    m_sessions[id] = std::shared_ptr<Session>(new Session{
        id, EntityType::NOT_ASSIGNED, "", true});
    std::cout << "[SM Log] Session added: " << id << "\n";
}

void SessionManager::removeSession(const int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->isValid = false;
        it->second->id = -1;
        m_sessions.erase(it);
        std::cout << "[SM Log] Session removed: " << id << "\n";
    }
}

void SessionManager::handleHandshake(const int id, const EntityType type, const std::string &app_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        if (m_sessions[id]->type == EntityType::NOT_ASSIGNED) {
            if (type == EntityType::DUMMY_SERVER) {
                m_sessions[id]->identity = app_id;
            }
            m_sessions[id]->type = type;
            std::cout << "[SM Log] Handshake successful: " << id << " " << type << "\n";
        }
    }
}

void SessionManager::displayConnections() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[SM Log] Active sessions:\n";
    bool found = false;
    for (auto &[id, session]: m_sessions) {
        found = true;
        if (session->type == EntityType::DUMMY_SERVER) {
            std::cout << "ID: " << id << "\nType: " << stringifyEntityType(session->type)
                  << "\nIdentity: " << session->identity << "\nisValid: " << session->isValid << "\n\n";
        }
        else {
            std::cout << "ID: " << id << "\nType: " << stringifyEntityType(session->type)
                  << "\nisLogged: " << session->ac_data->isLogged
                  << "\nIdentity: " << session->identity<< "\nisValid: " << session->isValid << "\nisInCodeState: "
                << session->ac_data->isInCodeState << "\n\n";
        }
    }
    if (!found) std::cout << "No active sessions.\n";
}

void SessionManager::logout(const int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->ac_data->secret_pairs.clear();
        it->second->ac_data->isLogged = false;
    }
}

void SessionManager::setIsLogged(const int id, const bool isLogged) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->ac_data->isLogged = isLogged;
        std::cout << "[SM Log] isLogged set to: " << isLogged
            << " for Session " << id << "\n";
    }
}

void SessionManager::setIsInCodeState(const int id, const bool isInCodeState) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->ac_data->isInCodeState = isInCodeState;
        std::cout << "[SM Log] isInCodeState set to: " << isInCodeState
            << " for Session " << id << "\n";
    }
}

void SessionManager::setIdentity(const int id, const std::string &identity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->identity = identity;
        std::cout << "[SM Log] Username set to: " << identity
            << " for Session " << id << "\n";
    }
}

void SessionManager::setSecretPairings(const int id, const std::map<std::string, std::string> &pairings) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->ac_data->secret_pairs = pairings;
    }
}

int SessionManager::getIDFromUsername(const std::string &username) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &[id, session]: m_sessions) {
        if (session->identity == username) return id;
    }
    return -1;
}

std::shared_ptr<Session> SessionManager::getSession(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second;
    return nullptr;
}

EntityType SessionManager::getEntityType(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second->type;
    return EntityType::NOT_ASSIGNED;
}

bool SessionManager::getIsLogged(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second->ac_data->isLogged;
    return false;
}

std::string SessionManager::getIdentity(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second->identity;
    return "";
}

std::vector<std::shared_ptr<Session>> SessionManager::getActiveSessions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Session>> sessions;

    for (const auto &session: m_sessions | std::views::values) {
        if (session->ac_data->isInCodeState) sessions.push_back(session);
    }
    return sessions;
}
