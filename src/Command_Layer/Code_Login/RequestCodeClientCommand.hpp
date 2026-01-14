#ifndef MY2FA_REQUESTCODECLIENTCOMMAND_HPP
#define MY2FA_REQUESTCODECLIENTCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class RequestCodeClientCommand : public Command {
public:
    explicit RequestCodeClientCommand(std::string appid);

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getAppid() const;

private:
    const std::string m_appid;
};

#endif //MY2FA_REQUESTCODECLIENTCOMMAND_HPP
