#ifndef MY2FA_REQUESTNOTIFICATIONSERVERCOMMAND_HPP
#define MY2FA_REQUESTNOTIFICATIONSERVERCOMMAND_HPP

#include <sstream>
#include <utility>
#include "../Base/Command.hpp"

class RequestNotificationServerCommand : public Command {
private:
    std::string uuid;
    const int appid;
public:
    RequestNotificationServerCommand(std::string uuid, const int appid)
        : uuid(std::move(uuid)), appid(appid) {}

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::REQ_NOTIF_SERVER) << DELIMITER << uuid << DELIMITER << appid;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override{
        return CommandType::REQ_NOTIF_SERVER;
    }

    [[nodiscard]] std::string getUuid() const {
        return uuid;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_REQUESTNOTIFICATIONSERVERCOMMAND_HPP