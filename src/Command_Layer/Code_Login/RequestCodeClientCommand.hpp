#ifndef MY2FA_REQUESTCODECLIENTCOMMAND_HPP
#define MY2FA_REQUESTCODECLIENTCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class RequestCodeClientCommand : public Command {
public:
    RequestCodeClientCommand(std::string uuid, int appid);

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getUuid() const;
    [[nodiscard]] int getAppid() const;

private:
    std::string m_uuid;
    const int m_appid;
};

#endif //MY2FA_REQUESTCODECLIENTCOMMAND_HPP
