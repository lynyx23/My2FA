#ifndef MY2FA_VALIDATECODECLIENTCOMMAND_HPP
#define MY2FA_VALIDATECODECLIENTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class ValidateCodeClientCommand : public Command {
private:
    const int code;
    std::string uuid;
public:
    ValidateCodeClientCommand(const int code, std::string uuid)
        : code(code), uuid(std::move(uuid)) {}

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::VALIDATE_CODE_CLIENT) << DELIMITER << code << DELIMITER << uuid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override{
        return CommandType::VALIDATE_CODE_CLIENT;
    }

    [[nodiscard]] int getCode() const {
        return code;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }
};

#endif //MY2FA_VALIDATECODECLIENTCOMMAND_HPP