#ifndef MY2FA_REQUESTNOTIFICATIONCLIENTCOMMAND_HPP
#define MY2FA_REQUESTNOTIFICATIONCLIENTCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class RequestNotificationClientCommand : public Command {
private:
    std::string uuid;

public:
    explicit RequestNotificationClientCommand(std::string uuid)
        : uuid(std::move(uuid)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::REQ_NOTIF_CLIENT) << DELIMITER << uuid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::REQ_NOTIF_CLIENT;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }
};

#endif //MY2FA_REQUESTNOTIFICATIONCLIENTCOMMAND_HPP
