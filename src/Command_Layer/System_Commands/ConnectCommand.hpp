#ifndef MY2FA_CONNCOMMAND_HPP
#define MY2FA_CONNCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"
#include "Command_Layer/Base/EntityType.hpp"

class ConnectCommand : public Command {
public:
    explicit ConnectCommand(EntityType connection_type);
    ConnectCommand(EntityType connection_type, std::string app_id);

    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;

    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] EntityType getConnectionType() const;

private:
    EntityType m_connection_type;
    std::string m_app_id;
};

#endif //MY2FA_CONNCOMMAND_HPP
