#ifndef MY2FA_CODERESPONSECOMMAND_HPP
#define MY2FA_CODERESPONSECOMMAND_HPP

#include <cstdint>
#include "Command_Layer/Base/Command.hpp"

class CodeResponseCommand : public Command {
public:
    CodeResponseCommand(const std::string &code, uint32_t remaining_time);

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getCode() const;
    [[nodiscard]] uint32_t getRemainingTime() const;

private:
    const std::string m_code;
    const uint32_t m_remaining_time;

};

#endif //MY2FA_CODERESPONSECOMMAND_HPP
