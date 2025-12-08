#ifndef MY2FA_COMMAND_HPP
#define MY2FA_COMMAND_HPP

#include <string>

class Command {
public:
    virtual std::string execute(std::string& args) = 0;
    virtual ~Command() = default;
};

#endif //MY2FA_COMMAND_HPP