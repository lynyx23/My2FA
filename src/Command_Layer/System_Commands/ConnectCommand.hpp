#ifndef MY2FA_CONNCOMMAND_HPP
#define MY2FA_CONNCOMMAND_HPP

#include <utility>
#include <sstream>

#include "../Base/Command.hpp"

class ConnectCommand : public Command {
private:
    std::string connection_type; // AUTH_CLIENT, AUTH_SERVER, DUMMY_CLIENT, DUMMY_SERVER
    std::string id;

public:
    ConnectCommand(std::string type, std::string connectionId)
        : connection_type(std::move(type)), id(std::move(connectionId)) {
    }

    [[nodiscard]] std::string execute() const override {
        std::ostringstream ss;
        ss << static_cast<int>(CommandType::CONN) << DELIMITER << connection_type << DELIMITER << id;
        return ss.str();
    }

    [[nodiscard]] CommandType getType() const override {
        return CommandType::CONN;
    }

    [[nodiscard]] std::string getId() const {
        return id;
    }

    [[nodiscard]] std::string getConnectionType() const {
        return connection_type;
    }
};

#endif //MY2FA_CONNCOMMAND_HPP
