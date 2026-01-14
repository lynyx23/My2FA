#ifndef MY2FA_TOTPMANAGER_HPP
#define MY2FA_TOTPMANAGER_HPP

#ifdef A_SERVER
#include <Command_Layer/Context.hpp>
#include <memory>
#include <thread>
#include "Session_Manager/SessionManager.hpp"

class TOTPManager {
public:
    explicit TOTPManager(Context &ctx);
    ~TOTPManager() = default;
    void start();
    void sendCodesToClient(const std::shared_ptr<Session> &session);
private:
    Context &m_ctx;
    std::jthread m_thread;
    void m_run(std::stop_token stop_token);
    [[nodiscard]] bool canReceiveCode(const std::shared_ptr<Session> &session) const;
};

#endif
#endif // MY2FA_TOTPMANAGER_HPP
