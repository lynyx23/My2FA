#include "SessionManager.hpp"
#include <iostream>
#include <memory>
#include <ranges>

void SessionManager::addSession(const int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // make_shared doesn't work because of the atomic bools which are un-copyable
    m_sessions[id] = std::shared_ptr<Session>(new Session{
        id, EntityType::NOT_ASSIGNED, "", "", false, true, false});
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

    std::cout << "[SM Log] Session removed: " << id << "\n";
}

void SessionManager::handleHandshake(const int id, const EntityType type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        if (m_sessions[id]->type == EntityType::NOT_ASSIGNED) {
            m_sessions[id]->type = type;
            std::cout << "[SM Log] Handshake successful: " << id << " " << type << "\n";
        }
    }
}

void SessionManager::displayConnections() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[SM Log] Active sessions:\n";
    for (auto &[id, session]: m_sessions) {
        std::cout << "ID: " << id << " | Type: " << stringifyEntityType(session->type) << " | UUID: " << session->uuid
                  << " | Secret: " << session->secret << "\nisLogged: " << session->isLogged
                  << " | isValid: " << session->isValid << " | isInCodeState: " << session->isInCodeState << "\n";
    }
}

bool SessionManager::checkLoggedIn(const std::string &uuid) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto &session: m_sessions | std::views::values) {
        if (session->uuid == uuid) return true;
    }
    return false;
}

void SessionManager::logout(const int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->uuid = "";
        it->second->secret = "";
        it->second->isLogged = false;
    }
}

void SessionManager::setIsLogged(const int id, const bool isLogged) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->isLogged = isLogged;
        std::cout << "[SM Log] isLogged set to: " << isLogged
            << "for Session " << id << "\n";
    }
}

void SessionManager::setUUID(const int id, const std::string &uuid) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->uuid = uuid;
        std::cout << "[SM Log] UUID set to: " << uuid
            << "for Session " << id << "\n";
    }
}

void SessionManager::setIsInCodeState(const int id, const bool isInCodeState) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->isInCodeState = isInCodeState;
        std::cout << "[SM Log] isInCodeState set to: " << isInCodeState
            << "for Session " << id << "\n";
    }
}

void SessionManager::setSecret(const int id, const std::string &secret) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (auto it = m_sessions.find(id); it != m_sessions.end()) {
        it->second->secret = secret;
        std::cout << "[SM Log] Secret set to: " << secret
            << "for Session " << id << "\n";
    }
}

int SessionManager::getID(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->first;
    return -1;
}

std::shared_ptr<Session> SessionManager::getSession(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second;
    return nullptr;
}

std::string SessionManager::getUUID(const int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (const auto it = m_sessions.find(id); it != m_sessions.end())
        return it->second->uuid;
    return "";
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
        return it->second->isLogged;
    return false;
}

std::vector<std::shared_ptr<Session>> SessionManager::getActiveSessions() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Session>> sessions;

    for (const auto &session: m_sessions | std::views::values) {
        if (session->isInCodeState) sessions.push_back(session);
    }
    return sessions;
}
