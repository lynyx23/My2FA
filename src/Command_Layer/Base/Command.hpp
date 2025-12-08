#ifndef MY2FA_COMMAND_HPP
#define MY2FA_COMMAND_HPP

#include <string>
#include "CommandTypes.hpp"

class Command {
public:
    virtual ~Command() = default;
    [[nodiscard]] virtual std::string serialize() const = 0; // Object -> String
    [[nodiscard]] virtual CommandType getType() const = 0; // Returns the type of Command from the enum class
};

#endif //MY2FA_COMMAND_HPP