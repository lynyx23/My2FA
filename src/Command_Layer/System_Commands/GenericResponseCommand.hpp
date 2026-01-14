#ifndef MY2FA_GENERICRESPONSECOMMAND_HPP
#define MY2FA_GENERICRESPONSECOMMAND_HPP

#pragma once
#include "Command_Layer/Base/Command.hpp"

class GenericResponseCommand : public Command {
public:
    GenericResponseCommand(CommandType type, bool resp, std::string message, std::string extra);
    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;
    [[nodiscard]] bool getResponse() const;
    [[nodiscard]] CommandType getType() const;
    [[nodiscard]] std::string getMessage() const;

private:
    const CommandType m_type;
    const bool m_resp;
    const std::string m_msg;
    const std::string m_extra;
};

#endif // MY2FA_GENERICRESPONSECOMMAND_HPP
