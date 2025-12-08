#include <vector>
#include <sstream>
#include "Base/Command.hpp"
#include "CommandFactory.hpp"

// Hleper function to split command string into command and argument tokens
static std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;
    while(std::getline(ss, token, DELIMITER)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::unique_ptr<Command> CommandFactory::create(const std::string &data) {
    const std::vector<std::string> tokens = split(data);

    if(tokens.empty()) return nullptr;
    int type_int = std::stoi(tokens[0]);
    switch (auto type_cmd = static_cast<CommandType>(type_int)) {
        case CommandType::CONN:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::ACK:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::PING:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::ERR:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::LOGIN_REQ:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::LOGIN_RESP:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::REQ_NOTIF_CLIENT:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::REQ_NOTIF_SERVER:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::SEND_NOTIF:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::NOTIF_RESP_CLIENT:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::NOTIF_RESP_SERVER:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::REQ_CODE_CLIENT:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::CODE_RESP:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::VALIDATE_CODE_CLIENT:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::VALIDATE_CODE_SERVER:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::VALIDATE_RESP_SERVER:
            if (tokens.size() == 3) {

            }
            break;
        case CommandType::VALIDATE_RESP_CLIENT:
            if (tokens.size() == 3) {

            }
            break;
        default:
            return nullptr; // invalid command
    }
    return nullptr; // in case the switch fails for some reason
}

