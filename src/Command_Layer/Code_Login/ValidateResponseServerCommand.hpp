#ifndef MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP
#define MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class ValidateResponseServerCommand : public Command {
private:
    const bool resp;
    std::string uuid;
    const int appid;

public:
    ValidateResponseServerCommand(const bool resp, std::string uuid, const int appid)
        : resp(resp), uuid(std::move(uuid)), appid(appid) {
    }

    [[nodiscard]] std::string execute() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_RESP_SERVER)
                << DELIMITER << static_cast<int>(resp) << DELIMITER << uuid << DELIMITER << appid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_RESP_SERVER;
    }

    [[nodiscard]] int getResp() const {
        return resp;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_VALIDATERESPONSESERVERCOMMAND_HPP
