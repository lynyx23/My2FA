#include <vector>
#include <sstream>
#include "Base/Command.hpp"
#include "CommandFactory.hpp"
#include "System_Commands/SystemCommands.hpp"
#include "Credential_Login/CredentialLoginCommands.hpp"
#include "Notification_Login/NotificationLoginCommands.hpp"
#include "Code_Login/CodeLoginCommands.hpp"

// Hleper function to split command string into command and argument tokens

std::unique_ptr<Command> CommandFactory::create(const std::string &data) {
    const std::vector<std::string> args = split(data);
    if(args.empty()) return nullptr;
    int type_int = std::stoi(args[0]);

    switch (auto type_cmd = static_cast<CommandType>(type_int)) {
        case CommandType::CONN:
            if (args.size() == 3) {
                return std::make_unique<ConnectCommand>(args[1], args[2]);
            }
            break;
        case CommandType::PING:
            if (args.size() == 1) {
                return std::make_unique<PingCommand>();
            }
            break;
        case CommandType::ERR:
            if (args.size() == 3) {
                return std::make_unique<ErrorCommand>(stoi(args[1]), args[2]);
            }
            break;
        case CommandType::LOGIN_REQ:
            if (args.size() == 3) {
                return std::make_unique<LoginRequestCommand>(args[1], args[2]);
            }
            break;
        case CommandType::LOGIN_RESP:
            if (args.size() == 3) {
                bool resp = (args[1] == "1");
                return std::make_unique<LoginResponseCommand>(resp, args[2]);
            }
            break;
        case CommandType::REQ_NOTIF_CLIENT:
            if (args.size() == 2) {
                return std::make_unique<RequestNotificationClientCommand>(args[1]);
            }
            break;
        case CommandType::REQ_NOTIF_SERVER:
            if (args.size() == 3) {
                return std::make_unique<RequestNotificationServerCommand>(args[1], stoi(args[2]));
            }
            break;
        case CommandType::SEND_NOTIF:
            if (args.size() == 2) {
                return std::make_unique<SendNotificationCommand>(stoi(args[1]));
            }
            break;
        case CommandType::NOTIF_RESP_CLIENT:
            if (args.size() == 3) {
                bool resp = (args[1] == "1");
                return std::make_unique<NotificationResponseClientCommand>(resp, stoi(args[2]));
            }
            break;
        case CommandType::NOTIF_RESP_SERVER:
            if (args.size() == 3) {
                bool resp = (args[1] == "1");
                return std::make_unique<NotificationResponseServerCommand>(resp, args[2]);
            }
            break;
        case CommandType::REQ_CODE_CLIENT:
            if (args.size() == 3) {
                return std::make_unique<RequestCodeClientCommand>(args[1], stoi(args[2]));
            }
            break;
        case CommandType::CODE_RESP:
            if (args.size() == 2) {
                return std::make_unique<CodeResponseCommand>(stoi(args[1]));
            }
            break;
        case CommandType::VALIDATE_CODE_CLIENT:
            if (args.size() == 3) {
                return std::make_unique<ValidateCodeClientCommand>(stoi(args[1]), args[2]);
            }
            break;
        case CommandType::VALIDATE_CODE_SERVER:
            if (args.size() == 4) {
                return std::make_unique<ValidateCodeServerCommand>(stoi(args[1]), args[2], stoi(args[3]));
            }
            break;
        case CommandType::VALIDATE_RESP_SERVER:
            if (args.size() == 4) {
                bool resp = (args[1] == "1");
                return std::make_unique<ValidateResponseServerCommand>(resp, args[2], stoi(args[3]));
            }
            break;
        case CommandType::VALIDATE_RESP_CLIENT:
            if (args.size() == 3) {
                bool resp = (args[1] == "1");
                return std::make_unique<ValidateResponseClientCommand>(resp, args[2]);
            }
            break;
        default:
            return nullptr; // invalid command
    }
    return nullptr; // in case the switch fails for some reason
}

