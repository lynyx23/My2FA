#ifndef MY2FA_NOTIFICATIONRESPONSESERVERCOMMAND_HPP
#define MY2FA_NOTIFICATIONRESPONSESERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class NotificationResponseServerCommand : public Command {
private:
    const bool response;
    std::string uuid;

public:
    NotificationResponseServerCommand(const bool response, std::string uuid)
        : response(response), uuid(std::move(uuid)) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::NOTIF_RESP_SERVER) << DELIMITER
                << static_cast<int>(response) << DELIMITER << uuid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::NOTIF_RESP_SERVER;
    }

    [[nodiscard]] bool getResponse() const {
        return response;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }
};

#endif //MY2FA_NOTIFICATIONRESPONSESERVERCOMMAND_HPP
