#ifndef MY2FA_PINGCOMMAND_HPP
#define MY2FA_PINGCOMMAND_HPP

#include <sstream>

#include "../Base/Command.hpp"

class PingCommand : public Command {
public:
    PingCommand() = default;

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::PING);
        return ss.str();
    }

    void execute(ServerContext &ctx, int client_fd) override {};

    [[nodiscard]] CommandType getType() const override {
        return CommandType::PING;
    }
};

#endif //MY2FA_PINGCOMMAND_HPP
