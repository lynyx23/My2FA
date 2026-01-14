#ifndef MY2FA_PAIRCOMMAND_HPP
#define MY2FA_PAIRCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class PairCommand : public Command {
public:
    explicit PairCommand(std::string d_username);
    PairCommand() = default;

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;

    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] std::string getCode() const;
    [[nodiscard]] std::string getD_username() const;

private:
    const std::string m_d_username;
    const std::string m_code;
};

#endif // MY2FA_PAIRCOMMAND_HPP
