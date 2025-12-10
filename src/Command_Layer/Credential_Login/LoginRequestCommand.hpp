#ifndef MY2FA_LOGINREQUESTCOMMAND_HPP
#define MY2FA_LOGINREQUESTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class LoginRequestCommand : public Command {
private:
    std::string username;
    std::string password;

public:
    LoginRequestCommand(std::string user, std::string pass)
        : username(std::move(user)), password(std::move(pass)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::LOGIN_REQ) << DELIMITER << username << DELIMITER << password;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::LOGIN_REQ;
    }

    [[nodiscard]] std::string getUsername() const {
        return username;
    }

    [[nodiscard]] std::string getPassword() const {
        return password;
    }
};

#endif //MY2FA_LOGINREQUESTCOMMAND_HPP
