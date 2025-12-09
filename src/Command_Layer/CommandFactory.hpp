#ifndef MY2FA_COMMANDFACTORY_HPP
#define MY2FA_COMMANDFACTORY_HPP
#include <memory>
#include <vector>
#include <sstream>
#include "Base/Command.hpp"

class CommandFactory {
    public:
        static std::unique_ptr<Command> create(const std::string &data);
};

static std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;
    while(std::getline(ss, token, DELIMITER)) {
        tokens.push_back(token);
    }
    return tokens;
}

#endif //MY2FA_COMMANDFACTORY_HPP