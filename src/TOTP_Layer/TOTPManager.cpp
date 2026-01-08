#include "TOTPManager.hpp"
#ifdef SERVER_SIDE
#include "TOTPGenerator.hpp"
#include "Command_Layer/Context.hpp"
#include "Session_Manager/SessionManager.hpp"
#include <chrono>
#include <iostream>
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Command_Layer/Code_Login/CodeResponseCommand.hpp"

TOTPManager::TOTPManager(Context &ctx)
    : m_ctx(ctx) {}

void TOTPManager::start() {
    if (m_thread.joinable()) return; // don't start if already running
    m_thread = std::jthread([this](std::stop_token stop_token) {
        this->m_run(stop_token);
    });
    std::cout << "[TM Log] Thread started!\n";
}

bool TOTPManager::canReceiveCode(const std::shared_ptr<Session> &session) const {
    // checks to see if the session is dead, user not logged in, or not in showcode state
    // or if the user doesn't have 2fa setup
    return session && session->isValid && session->isLogged
        && session->isInCodeState && !session->secret.empty();
}

void TOTPManager::m_run(std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
        uint32_t sleep = TOTPGenerator::getRemainingSeconds();

        // std::cout << "[TM Log] Sleeping for " << sleep << " seconds...\n";
        for (uint32_t i = 0; i < sleep; ++i) {
            if (stop_token.stop_requested()) return;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        // std::cout << "[TM Log] Sleep over!\n";

        if (stop_token.stop_requested()) return;

        for (auto sessions = m_ctx.session_manager.getActiveSessions();
            const auto &session : sessions) {
            if (!canReceiveCode(session)) continue;
            sendCodeToClient(session);
        }
    }
}

    void TOTPManager::sendCodeToClient(const std::shared_ptr<Session> &session) {
        if (!canReceiveCode(session)) {
            std::cerr << "[TM Error] Invalid session ("<< session->id << ")!\n";
            return;
        }
        std::string totp = TOTPGenerator::generateTOTP(session->secret);
        uint32_t timeRemaining = TOTPGenerator::getRemainingSeconds();

        // std::cout << "[DEBUG] Sending CodeResp to Client\n";
        m_ctx.server_handler.sendCommand(session->id,
            std::make_unique<CodeResponseCommand>(totp, timeRemaining));
        // std::cout << "[DEBUG] Sent CodeResp!\n";
    }

#endif


