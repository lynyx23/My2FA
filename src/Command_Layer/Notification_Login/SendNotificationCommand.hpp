#ifndef MY2FA_SENDNOTIFICATIONCOMMAND_HPP
#define MY2FA_SENDNOTIFICATIONCOMMAND_HPP

#include <sstream>
#include "../Base/Command.hpp"

class SendNotificationCommand : public Command {
private:
    const int appid;

public:
    explicit SendNotificationCommand(const int appid) : appid(appid) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::SEND_NOTIF) << DELIMITER << appid;
        return ss.str();
    };

    [[nodiscard]] CommandType getType() const override {
        return CommandType::SEND_NOTIF;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_SENDNOTIFICATIONCOMMAND_HPP
