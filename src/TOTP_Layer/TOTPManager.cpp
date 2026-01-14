#include "TOTPManager.hpp"
#ifdef A_SERVER
#include <chrono>
#include <iostream>
#include <sstream>
#include "TOTPGenerator.hpp"

#include "Command_Layer/Code_Login/CodeResponseCommand.hpp"
#include "Command_Layer/Context.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"

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
    return session && session->isValid && session->ac_data->isLogged
        && session->ac_data->isInCodeState && !session->ac_data->secret_pairs.empty();
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
            sendCodesToClient(session);
        }
    }
}

void TOTPManager::sendCodesToClient(const std::shared_ptr<Session> &session) {
    if (!canReceiveCode(session)) {
        std::cerr << "[TM Error] Invalid session ("<< session->id << ")!\n";
        return;
    }

    constexpr char PAIR_DELIMITER = '|';
    constexpr char CODE_DELIMITER = ':';

    std::stringstream ss;
    bool first = true;
    for (const auto &[app_id, secret] : session->ac_data->secret_pairs) {
        if (!first) ss << PAIR_DELIMITER;
        first = false;
        ss << app_id << CODE_DELIMITER << TOTPGenerator::generateTOTP(secret);
    }
    uint32_t timeRemaining = TOTPGenerator::getRemainingSeconds();
    m_ctx.server_handler.sendCommand(session->id,
            std::make_unique<CodeResponseCommand>(timeRemaining, ss.str()));
}

#endif


