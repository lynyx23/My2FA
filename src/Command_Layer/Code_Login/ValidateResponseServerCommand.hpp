#ifndef MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP
#define MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "Command_Layer/Base/Command.hpp"
#include "Command_Layer/System_Commands/GenericResponseCommand.hpp"
#ifdef D_SERVER
#include <iostream>
#include "Command_Layer/Context.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#endif


class ValidateResponseServerCommand : public Command {
public:
    ValidateResponseServerCommand(const bool resp, std::string username, std::string appid)
        : m_resp(resp), m_username(std::move(username)), m_app_id(std::move(appid)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_RESP_SERVER)
                << DELIMITER << static_cast<int>(m_resp) << DELIMITER << m_username << DELIMITER << m_app_id;
        return ss.str();
    }

    void execute(Context &ctx, int client_fd) override {
    #ifdef D_SERVER
        ctx.server_handler.sendCommand(ctx.session_manager.getIDFromUsername(m_username),
            std::make_unique<GenericResponseCommand>(CommandType::CODE_CHK_RESP, m_resp, "", m_username));
        std::cout << "[DEBUG] 2FA Check was " << (m_resp ? "successful" : "unsuccessful") << "!\n";
    #endif
    };

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_RESP_SERVER;
    }

    [[nodiscard]] bool getResp() const {
        return m_resp;
    }

    [[nodiscard]] std::string getUuid() const {
        return m_username;
    }

    [[nodiscard]] std::string getAppid() const {
        return m_app_id;
    }

private:
    const bool m_resp;
    std::string m_username;
    const std::string m_app_id;
};

#endif //MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP
