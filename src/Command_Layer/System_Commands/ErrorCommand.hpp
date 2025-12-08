#ifndef MY2FA_ERRCOMMAND_HPP
#define MY2FA_ERRCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class ErrorCommand : public Command {
private:
    const int code;
    std::string msg;

public:
    ErrorCommand(const int errCode, std::string message)
        : code(errCode), msg(std::move(message)) {}

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::ERR) << DELIMITER << code << DELIMITER << msg;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override{
        return CommandType::ERR;
    }

    [[nodiscard]] int getCode() const {
        return code;
    }

    [[nodiscard]] std::string getMessage() const {
        return msg;
    }
};

#endif //MY2FA_ERRCOMMAND_HPP