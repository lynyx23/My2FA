#ifndef MY2FA_ERRCOMMAND_HPP
#define MY2FA_ERRCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class ErrorCommand : public Command {
public:
    ErrorCommand(int errCode, std::string message);
    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;
    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] int getCode() const;
    [[nodiscard]] std::string getMessage() const;

private:
    const int m_code;
    std::string m_msg;
};

#endif // MY2FA_ERRCOMMAND_HPP
