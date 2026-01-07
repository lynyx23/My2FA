#ifndef MY2FA_LOGOUTREQUESTCOMMAND_HPP
#define MY2FA_LOGOUTREQUESTCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class LogoutRequestCommand : public Command {
public:
    explicit LogoutRequestCommand(std::string uuid);

    [[nodiscard]] std::string serialize() const override;

    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;

    [[nodiscard]] std::string getUuid() const;

private:
    std::string m_uuid;
};

#endif // MY2FA_LOGOUTREQUESTCOMMAND_HPP
