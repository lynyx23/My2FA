#ifndef MY2FA_VALIDATERESPONSECLIENTCOMMAND_HPP
#define MY2FA_VALIDATERESPONSECLIENTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class ValidateResponseClientCommand : public Command {
private:
    const bool resp;
    std::string uuid;

public:
    ValidateResponseClientCommand(const bool resp, std::string uuid)
        : resp(resp), uuid(std::move(uuid)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_RESP_CLIENT)
                << DELIMITER << static_cast<int>(resp) << DELIMITER << uuid;
        return ss.str();
    }

    void execute(ServerContext &ctx, int client_fd) override {};

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_RESP_CLIENT;
    }

    [[nodiscard]] int getResp() const {
        return resp;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }
};

#endif //MY2FA_VALIDATERESPONSECLIENTCOMMAND_HPP
