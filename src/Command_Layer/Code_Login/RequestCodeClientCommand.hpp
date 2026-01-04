#ifndef MY2FA_REQUESTCODECLIENTCOMMAND_HPP
#define MY2FA_REQUESTCODECLIENTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "Command_Layer/Base/Command.hpp"

class RequestCodeClientCommand : public Command {
public:
    RequestCodeClientCommand(std::string uuid, const int appid)
        : uuid(std::move(uuid)), appid(appid) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::REQ_CODE_CLIENT) << DELIMITER << uuid << DELIMITER << appid;
        return ss.str();
    }

    void execute(ServerContext &ctx, int client_fd) override {};

    [[nodiscard]] CommandType getType() const override {
        return CommandType::REQ_CODE_CLIENT;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }

private:
    std::string uuid;
    const int appid;

};

#endif //MY2FA_REQUESTCODECLIENTCOMMAND_HPP
