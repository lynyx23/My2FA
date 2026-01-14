#ifndef MY2FA_VALIDATECODESERVERCOMMAND_HPP
#define MY2FA_VALIDATECODESERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "Command_Layer/Base/Command.hpp"
#ifdef A_SERVER
#include "Command_Layer/Context.hpp"
#include "TOTP_Layer/TOTPGenerator.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Database_Layer/Database.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "ValidateResponseServerCommand.hpp"
#endif

class ValidateCodeServerCommand : public Command {
public:
    ValidateCodeServerCommand(std::string code, std::string username, std::string appid)
        : m_code(std::move(code)), m_username(std::move(username)), m_app_id(std::move(appid)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_CODE_SERVER)
                << DELIMITER << m_code << DELIMITER << m_username << DELIMITER << m_app_id;
        return ss.str();
    }

    //TODO reimplement login so that you have to get through 2fa to be logged in; maybe not on A-side

    void execute(Context &ctx, const int client_fd) override {
    #ifdef A_SERVER
        bool resp;
        auto result = Database::getSecret(m_username, m_app_id);
        if (result.has_value() && TOTPGenerator::verifyCode(result.value(), m_code)) {
            resp = true;
        } else resp = false;

        ctx.server_handler.sendCommand(client_fd,
                std::make_unique<ValidateResponseServerCommand>(resp, m_username, m_app_id));
    #endif
    };

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_CODE_SERVER;
    }

    [[nodiscard]] std::string getCode() const {
        return m_code;
    }

    [[nodiscard]] std::string getUuid() const {
        return m_username;
    }

    [[nodiscard]] std::string getAppID() const {
        return m_app_id;
    }

private:
    const std::string m_code;
    std::string m_username;
    const std::string m_app_id;
};

#endif //MY2FA_VALIDATECODESERVERCOMMAND_HPP