#ifndef MY2FA_EXITSCSCOMMAND_HPP
#define MY2FA_EXITSCSCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"

class ExitSCSCommand : public Command {
public:
    ExitSCSCommand() = default;

    [[nodiscard]] std::string serialize() const override;

    void execute(Context &ctx, int client_fd) override;

    [[nodiscard]] CommandType getType() const override;
};

#endif // MY2FA_EXITSCSCOMMAND_HPP
