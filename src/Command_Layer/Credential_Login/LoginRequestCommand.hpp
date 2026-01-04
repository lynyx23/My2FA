#ifndef MY2FA_LOGINREQUESTCOMMAND_HPP
#define MY2FA_LOGINREQUESTCOMMAND_HPP

#pragma once
#include "Command_Layer/Base/Command.hpp"

class LoginRequestCommand : public Command {
public:
    LoginRequestCommand(std::string user, std::string pass);
    [[nodiscard]] std::string serialize() const override;
    void execute(ServerContext &ctx, int client_fd) override;
    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getUsername() const;
    [[nodiscard]] std::string getPassword() const;

private:
    std::string m_username;
    std::string m_password;
};

#endif //MY2FA_LOGINREQUESTCOMMAND_HPP
