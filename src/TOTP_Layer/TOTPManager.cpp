#include "TOTPManager.hpp"
#ifdef SERVER_SIDE
#include "TOTPGenerator.hpp"
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#include <chrono>
#include <iostream>

TOTPManager::TOTPManager(Context &ctx)
    : m_ctx(ctx) {}

void TOTPManager::start() {
    if (m_thread.joinable()) return; // don't start if already running
    m_thread = std::jthread([this](std::stop_token stop_token) {
        this->m_run(stop_token);
    });
    std::cout << "[TM Log] Thread started!\n";
}

void TOTPManager::m_run(std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        uint32_t sleep = TOTPGenerator::getRemainingSeconds();

        for (uint32_t i = 0; i < sleep; ++i) {
            if (stop_token.stop_requested()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        if (stop_token.stop_requested()) return;

        for (auto sessions = m_ctx.session_manager.getAllSessions();
            const auto &session : sessions) {
            // if the session is dead or if the user hasn't passed the first login don't run
            if (!session->isValid || !session->isLogged) continue;
            // if user hasn't registered 2fa
            if (session->secret.empty()) continue;

            const std::string totp = TOTPGenerator::generateTOTP(session->secret);
            if (!session->isValid) continue;

        }
    }
}

#endif


