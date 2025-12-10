#ifndef MY2FA_REQUESTCODECLIENTCOMMAND_HPP
#define MY2FA_REQUESTCODECLIENTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class RequestCodeClientCommand : public Command {
private:
    std::string uuid;
    const int appid;

public:
    RequestCodeClientCommand(std::string uuid, const int appid)
        : uuid(std::move(uuid)), appid(appid) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::REQ_CODE_CLIENT) << DELIMITER << uuid << DELIMITER << appid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::REQ_CODE_CLIENT;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_REQUESTCODECLIENTCOMMAND_HPP
