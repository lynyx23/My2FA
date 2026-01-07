#ifndef MY2FA_CONNCOMMAND_HPP
#define MY2FA_CONNCOMMAND_HPP

#include "Command_Layer/Base/Command.hpp"
#include "Command_Layer/Base/EntityType.hpp"

class ConnectCommand : public Command {
public:
    explicit ConnectCommand(EntityType type);
    [[nodiscard]] std::string serialize() const override;
    void execute(Context &ctx, int fd) override;
    [[nodiscard]] CommandType getType() const override;
    [[nodiscard]] EntityType getConnectionType() const;

private:
    EntityType m_connection_type;

};

#endif //MY2FA_CONNCOMMAND_HPP
