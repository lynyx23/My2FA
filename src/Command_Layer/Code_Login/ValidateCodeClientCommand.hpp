#ifndef MY2FA_VALIDATECODECLIENTCOMMAND_HPP
#define MY2FA_VALIDATECODECLIENTCOMMAND_HPP

#include <memory>
#include <sstream>
#include <utility>

#include "Command_Layer/Base/Command.hpp"
#ifdef A_SERVER
#include "Command_Layer/Context.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Auth_Layer/AuthManager.hpp"
#elif defined(D_SERVER)
#include "Command_Layer/Context.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "ValidateCodeServerCommand.hpp"
#endif

class ValidateCodeClientCommand : public Command {
public:
    explicit ValidateCodeClientCommand(std::string code)
        : m_code(std::move(code)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_CODE_CLIENT) << DELIMITER << m_code;
        return ss.str();
    }

    void execute(Context &ctx, const int client_fd) override {
#ifdef A_SERVER
        if (m_code.length() != 6) {
            const auto result = ctx.auth_manager->finishPairing(
            ctx.session_manager.getIdentity(client_fd), m_code);
            if (result.has_value()) {
                ctx.session_manager.getSession(client_fd)->ac_data->secret_pairs[result->first] = result->second;
            } else {
                std::cerr << "[2FA Pairing Error] Invalid token!\n";
            }
        }
#elif defined(D_SERVER)
        std::cout << "[2FA Check] Sending code to AS :" << m_code << "\n";
        ctx.client_handler->sendCommand(std::make_unique<ValidateCodeServerCommand>(m_code,
            ctx.session_manager.getIdentity(client_fd), ctx.app_id));
#endif
    };

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_CODE_CLIENT;
    }

    [[nodiscard]] std::string getCode() const {
        return m_code;
    }

private:
    const std::string m_code;
};

#endif //MY2FA_VALIDATECODECLIENTCOMMAND_HPP
