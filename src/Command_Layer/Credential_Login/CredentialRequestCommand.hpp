#ifndef MY2FA_CREDENTIALREQUESTCOMMAND_HPP
#define MY2FA_CREDENTIALREQUESTCOMMAND_HPP

#pragma once
#include "Command_Layer/Base/Command.hpp"

class CredentialRequestCommand : public Command {
public:
    CredentialRequestCommand(CommandType type, std::string user, std::string pass);
    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;
    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getUsername() const;
    [[nodiscard]] std::string getPassword() const;

private:
    CommandType m_type;
    std::string m_username;
    std::string m_password;
};

#endif // MY2FA_CREDENTIALREQUESTCOMMAND_HPP
