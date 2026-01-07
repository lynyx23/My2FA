#ifndef MY2FA_COMMAND_HPP
#define MY2FA_COMMAND_HPP

#include <string>
#include "CommandTypes.hpp"

struct Context; // forward declaration so that clients don't freak out over this

class Command {
public:
    virtual ~Command() = default;

    [[nodiscard]] virtual std::string serialize() const = 0; // object to string
    virtual void execute(Context &ctx, int fd) = 0; // processes a received command
    [[nodiscard]] virtual CommandType getType() const = 0; // Returns the type of Command from the enum class
};

#endif //MY2FA_COMMAND_HPP
