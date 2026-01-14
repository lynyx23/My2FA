#ifndef MY2FA_REQUESTNOTIFICATIONCOMMAND_HPP
#define MY2FA_REQUESTNOTIFICATIONCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class RequestNotificationCommand : public Command {
public:
    explicit RequestNotificationCommand(std::string username, std::string app_id = "");

    [[nodiscard]] std::string serialize() const override;

    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;

private:
    const std::string m_username;
    const std::string m_app_id;
};

#endif // MY2FA_REQUESTNOTIFICATIONCOMMAND_HPP
