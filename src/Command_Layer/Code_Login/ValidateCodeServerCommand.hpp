#ifndef MY2FA_VALIDATECODESERVERCOMMAND_HPP
#define MY2FA_VALIDATECODESERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class ValidateCodeServerCommand : public Command {
private:
    const int code;
    std::string uuid;
    const int appid;

public:
    ValidateCodeServerCommand(const int code, std::string uuid, const int appid)
        : code(code), uuid(std::move(uuid)), appid(appid) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_CODE_SERVER)
                << DELIMITER << code << DELIMITER << uuid << DELIMITER << appid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::VALIDATE_CODE_SERVER;
    }

    [[nodiscard]] int getCode() const {
        return code;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_VALIDATECODESERVERCOMMAND_HPP