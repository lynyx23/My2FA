#ifndef MY2FA_TOTPMANAGER_HPP
#define MY2FA_TOTPMANAGER_HPP

#ifdef SERVER_SIDE
#include <Command_Layer/Context.hpp>
#include <thread>

class TOTPManager {
public:
    explicit TOTPManager(Context &ctx);
    ~TOTPManager() = default;
    void start();
private:
    Context &m_ctx;
    std::jthread m_thread;
    void m_run(std::stop_token stop_token);
};

#endif
#endif // MY2FA_TOTPMANAGER_HPP
