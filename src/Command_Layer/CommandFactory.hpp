#ifndef MY2FA_COMMANDFACTORY_HPP
#define MY2FA_COMMANDFACTORY_HPP
#include <memory>
#include <string>
#include "Base/Command.hpp"

class CommandFactory {
    public:
        static std::unique_ptr<Command> create(const std::string &data);
};

#endif //MY2FA_COMMANDFACTORY_HPP