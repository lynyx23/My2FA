#ifndef MY2FA_CODERESPONSECOMMAND_HPP
#define MY2FA_CODERESPONSECOMMAND_HPP

#include <cstdint>
#include <map>
#include "Command_Layer/Base/Command.hpp"

class CodeResponseCommand : public Command {
public:
    CodeResponseCommand(uint32_t remaining_time, std::string payload);

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;

private:
    const uint32_t m_remaining_time;
    const std::string m_payload;
};

#endif //MY2FA_CODERESPONSECOMMAND_HPP
