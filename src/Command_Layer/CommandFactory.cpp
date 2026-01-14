#include "CommandFactory.hpp"
#include <iostream>
#include <string>
#include <vector>
#include "Base/Command.hpp"
#include "Base/EntityType.hpp"
#include "Code_Login/CodeLoginCommands.hpp"
#include "Code_Login/ExitSCSCommand.hpp"
#include "Credential_Login/CredentialLoginCommands.hpp"
#include "Notification_Login/NotificationLoginCommands.hpp"
#include "System_Commands/GenericResponseCommand.hpp"
#include "System_Commands/PairCommand.hpp"
#include "System_Commands/SystemCommands.hpp"

// Hleper function to split command string into command and argument tokens
// TODO conn timeout kick?
std::unique_ptr<Command> CommandFactory::create(const std::string &data) {
    const std::vector<std::string> args = split(data);
    if (args.empty()) return nullptr;
    int type_int;
    try {
        type_int = std::stoi(args[0]);
    } catch (...) {
        return nullptr;
    }

    switch (auto type_cmd = static_cast<CommandType>(type_int)) {
        case CommandType::CONN:
            if (args.size() == 2) {
                try {
                    int type = std::stoi(args[1]);
                    return std::make_unique<ConnectCommand>(static_cast<EntityType>(type));
                } catch (std::exception &e) {
                    std::cerr << "[CF Error] CONN - Invalid type: " << e.what() << "\n";
                    return nullptr;
                }
            }
            if (args.size() == 3) {
                try {
                    int type = std::stoi(args[1]);
                    return std::make_unique<ConnectCommand>(static_cast<EntityType>(type), args[2]);
                } catch (std::exception &e) {
                    std::cerr << "[CF Error] CONN - Invalid type: " << e.what() << "\n";
                    return nullptr;
                }
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
        case CommandType::PAIR_REQ:
#ifdef D_SERVER
            if (args.size() == 1) {
                return std::make_unique<PairCommand>();
            }
#elif defined(A_SERVER)
            if (args.size() == 2) {
                return std::make_unique<PairCommand>(args[1]);
            }
#endif
        case CommandType::CRED_REQ:
            if (args.size() == 4) {
                switch (const auto type = static_cast<CommandType>(stoi(args[1]))) {
                    case CommandType::LOGIN_REQ:
                    case CommandType::REGISTER_REQ:
                        return std::make_unique<CredentialRequestCommand>(type, args[2], args[3]);
                    default:
                        std::cerr << "[CF Error] Invalid credential request type: " << type << "\n";
                }
            }
        case CommandType::RESP:
            if (args.size() == 4 || args.size() == 5) {
                bool resp = (args[2] == "1");
                std::string extra = (args.size() == 5) ? args[4] : "";
                switch (const auto type = static_cast<CommandType>(stoi(args[1]))) {
                    case CommandType::LOGIN_RESP:
                    case CommandType::REGISTER_RESP:
                    case CommandType::PAIR_RESP:
                    case CommandType::CODE_CHK_RESP:
                    case CommandType::NOTIF_LOGIN_RESP:
                    case CommandType::NOTIF_RESP:
                        return std::make_unique<GenericResponseCommand>(type, resp, args[3], extra);
                    default:
                        std::cerr << "[CF Error] Invalid response type: " << type << "\n";
                }
            }
            break;
        case CommandType::LOGOUT_REQ:
                return std::make_unique<LogoutRequestCommand>();
        case CommandType::SEND_NOTIF:
            if (args.size() == 3) {
                return std::make_unique<SendNotificationCommand>(args[1], args[2]);
            }
            break;
        case CommandType::REQ_NOTIF:
            if (args.size() == 2) {
                return std::make_unique<RequestNotificationCommand>(args[1]);
            }
            if (args.size() == 3) {
                return std::make_unique<RequestNotificationCommand>(args[1], args[2]);
            }
            break;
        case CommandType::REQ_CODE_CLIENT:
            if (args.size() == 1) {
                return std::make_unique<RequestCodeClientCommand>();
            }
            break;
        case CommandType::CODE_RESP:
            if (args.size() == 3) {
                return std::make_unique<CodeResponseCommand>(stoi(args[1]), args[2]);
            }
            break;
        case CommandType::VALIDATE_CODE_CLIENT:
            if (args.size() == 2) {
                return std::make_unique<ValidateCodeClientCommand>(args[1]);
            }
            break;
        case CommandType::VALIDATE_CODE_SERVER:
            if (args.size() == 4) {
                return std::make_unique<ValidateCodeServerCommand>(args[1], args[2], args[3]);
            }
            break;
        case CommandType::VALIDATE_RESP_SERVER:
            if (args.size() == 4) {
                bool resp = (args[1] == "1");
                return std::make_unique<ValidateResponseServerCommand>(resp, args[2], args[3]);
            }
            break;
        case CommandType::EXIT_SCS:
            return std::make_unique<ExitSCSCommand>();
        default:
            return nullptr; // invalid command
    }
    return nullptr; // in case the switch fails for some reason
}

