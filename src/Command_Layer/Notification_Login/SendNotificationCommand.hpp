#ifndef MY2FA_SENDNOTIFICATIONCOMMAND_HPP
#define MY2FA_SENDNOTIFICATIONCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class SendNotificationCommand : public Command {
public:
    SendNotificationCommand(std::string reqID, std::string app_id);

    [[nodiscard]] std::string serialize() const override;

    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;

private:
    const std::string m_req_id;
    const std::string m_app_id;
};

#endif // MY2FA_SENDNOTIFICATIONCOMMAND_HPP
