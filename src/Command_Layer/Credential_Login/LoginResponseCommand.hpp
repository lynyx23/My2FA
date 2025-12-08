#ifndef MY2FA_LOGINRESPONSECOMMAND_HPP
#define MY2FA_LOGINRESPONSECOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class LoginResponseCommand : public Command {
private:
    const bool response;
    std::string uuid;
public:
    LoginResponseCommand(const bool resp, std::string uuid)
        : response(resp), uuid(std::move(uuid)) {}

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::LOGIN_RESP) << DELIMITER
            << static_cast<int>(response) << DELIMITER << uuid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override{
        return CommandType::LOGIN_RESP;
    }

    [[nodiscard]] bool getResponse() const {
        return response;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }
};

#endif //MY2FA_LOGINRESPONSECOMMAND_HPP