#ifndef MY2FA_NOTIFICATIONRESPONSECLIENTCOMMAND_HPP
#define MY2FA_NOTIFICATIONRESPONSECLIENTCOMMAND_HPP

#include <sstream>
#include "../Base/Command.hpp"

class NotificationResponseClientCommand : public Command {
private:
    const bool response;
    const int appid;

public:
    NotificationResponseClientCommand(const bool response, const int appid)
        : response(response), appid(appid) {
    }

    [[nodiscard]] std::string serialize() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::NOTIF_RESP_CLIENT) << DELIMITER
                << static_cast<int>(response) << DELIMITER << appid;
        return ss.str();
    }

    void execute(ServerContext &ctx, int client_fd) override {};

    [[nodiscard]] CommandType getType() const override {
        return CommandType::NOTIF_RESP_CLIENT;
    }

    [[nodiscard]] bool getResponse() const {
        return response;
    }

    [[nodiscard]] int getAppid() const {
        return appid;
    }
};

#endif //MY2FA_NOTIFICATIONRESPONSECLIENTCOMMAND_HPP
