#ifndef MY2FA_CODERESPONSECOMMAND_HPP
#define MY2FA_CODERESPONSECOMMAND_HPP

#include <sstream>
#include "../Base/Command.hpp"

class CodeResponseCommand : public Command {
private:
    const int code;

public:
    explicit CodeResponseCommand(const int code) : code(code) {
    }

    [[nodiscard]] std::string execute() const override {
        std::stringstream ss;
        ss << static_cast<int>(CommandType::CODE_RESP) << DELIMITER << code;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::CODE_RESP;
    }

    [[nodiscard]] int getCode() const {
        return code;
    }
};

#endif //MY2FA_CODERESPONSECOMMAND_HPP
