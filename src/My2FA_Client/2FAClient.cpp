#include <iomanip>
#include <memory>
#include <thread>
#include "Command_Layer/Base/EntityType.hpp"
#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Command_Layer/Code_Login/ExitSCSCommand.hpp"
#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Context.hpp"
#include "Command_Layer/Credential_Login/CredentialRequestCommand.hpp"
#include "Command_Layer/Credential_Login/LogoutRequestCommand.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/System_Commands/PairCommand.hpp"
#include "Command_Layer/System_Commands/SystemCommands.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"

#define PORT 27701

bool checkConsoleInput() {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    timeval timeout{0, 0};
    return select(STDIN_FILENO + 1, &read_set, nullptr, nullptr, &timeout) > 0;
}

uint8_t printCodeState(const uint32_t remainingSeconds, std::map<std::string, std::string> &codes) {
    constexpr uint8_t width = 30;
    uint8_t lines = 0;

    std::cout << "\r[";
    for (uint8_t i = 0; i < width; i++) {
        if (i < remainingSeconds) {
            std::cout << "\033[1;92m■\033[0m";
        } else std::cout << " ";
    }

    std::cout << "] " << remainingSeconds << "s\033[K\n";
    lines++;

    if (codes.empty()) {
        std::cout << "   (Waiting for codes...)\033[K\n";
        lines++;
    } else {
        for (const auto &[app_id, code] : codes) {
            std::cout << "  " << std::left << std::setw(15) << app_id
                      << ": [\033[1;92m" << code << "\033[0m]   \033[K\n";
            lines++;
        }
    }

    std::cout << std::flush;
    return lines;
}
//TODO polish interface
void runCodeState(Context &ctx, ClientConnectionHandler &handler) {
    std::cout << "\n[Live TOTP View - Press ENTER to return]\n" << std::flush;

    ctx.codeState = true;
    int last_height = 0;

    while (ctx.codeState && handler.isRunning()) {
        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            ctx.codeState = false;
            break;
        }

        handler.update();

        if (last_height > 0) {
            std::cout << "\033[" << last_height << "A";
        }

        const auto remaining = static_cast<uint32_t>(std::difftime(ctx.timeExpiration, std::time(nullptr)));
        last_height = printCodeState(remaining, ctx.codes);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    ctx.client_handler->sendCommand(std::make_unique<ExitSCSCommand>());
    std::cout << "\n\033[2J[Exiting TOTP view]\n" << std::flush;
}

void handleUserInput(Context &ctx, ClientConnectionHandler *handler, const std::string &input) {
    auto args = split(input);
    if (args.empty())
        return;

    std::unique_ptr<Command> command = nullptr;
    // note: switch won't work with strings
    if (args[0] == "help") {
        if (!ctx.isConnected) {
            std::cout << std::flush
                      << "  help         : Show this menu\n"
                      << "  reconnect    : Manually reconnect to AuthServer\n"
                      << "  exit         : Quit\n";
        }
        else if (!ctx.isLogged) {
            std::cout << std::flush
                      << "  help                          : Show this menu\n"
                      << "  login <user> <password>       : Login to AuthServer (e.g. login;user;pass)\n"
                      << "  register <user> <password>    : Register a new user on AuthServer (e.g. register;user;pass)\n"
                      << "  exit                          : Quit\n";
        }
        else {
            std::cout << std::flush
                      << "  help                          : Show this menu\n"
                      << "  login <user> <password>       : Login to AuthServer (e.g. login;user;pass)\n"
                      << "  logout                        : Logs out current user (e.g. logout)\n"
                      << "  register <user> <password>    : Register a new user on AuthServer (e.g. register;user;pass)\n"
                      << "  register                      : Register new user (e.g. register;user;pass)\n"
                      << "  code <uuid> <appid>           : Request 2FA Code (e.g. code;101)\n"
                      << "  accept <appid>                : Accept Notification (e.g. accept;101)\n"
                      << "  refuse <appid>                : Refuse Notification (e.g. refuse;101)\n"
                      << "  exit                          : Quit\n";
        }
        return;
    } else if (args[0] == "reconnect" || args[0] == "exit")
        return;
    else if (args[0] == "conn")
        command = std::make_unique<ConnectCommand>(EntityType::AUTH_CLIENT);
    else if (args[0] == "accept") {
        if (args.size() < 2) {
            std::cerr << "[AC Error] Incorrect format: accept;<appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(true, std::stoi(args[1]));
    } else if (args[0] == "refuse") {
        if (args.size() < 2) {
            std::cerr << "[Ac Error] Incorrect format: refuse;<appid>\n ";
            return;
        }
        command = std::make_unique<NotificationResponseClientCommand>(false, std::stoi(args[1]));
    } else if (args[0] == "pair") {
        if (args.size() != 2) {
            std::cerr << "[AC Error] Incorrect format: pair;<token>\n ";
            return;
        }
        command = std::make_unique<ValidateCodeClientCommand>(args[1]);
    } else if (args[0] == "code") {
        if (args.size() != 2) {
            std::cerr << "[AC Error] Incorrect format: code;<appid>\n ";
            return;
        }
        try {
            command = std::make_unique<RequestCodeClientCommand>(args[1]);
            if (handler && handler->isRunning()) {
                handler->sendCommand(command);
                runCodeState(ctx, *handler);
            }
            command = nullptr;
        } catch (...) {
            std::cerr << "[AC Error] Code must be an integer.\n";
            return;
        }
    } else if (args[0] == "login") {
        if (args.size() != 3) {
            std::cerr << "[AC Error] Incorrect format: login;<user>;<pass>\n ";
            return;
        }
        command = std::make_unique<CredentialRequestCommand>(CommandType::LOGIN_REQ, args[1], args[2]);
    } else if (args[0] == "logout") {
        ctx.username = "";
        ctx.isLogged = false;
        command = std::make_unique<LogoutRequestCommand>();
    } else if (args[0] == "register") {
        if (args.size() != 3) {
            std::cerr << "[AC Error] Incorrect format: register;<user>;<pass>\n ";
            return;
        }
        command = std::make_unique<CredentialRequestCommand>(CommandType::REGISTER_REQ, args[1], args[2]);
    }  else {
        std::cout << "[AC Error] Unknown command! Type help.\n ";
        return;
    }

    if (command) {
        const std::string data = command->serialize();
        if (handler && handler->isRunning()) {
            handler->sendCommand(command);
            std::cout << "[AC -> AS] Sent: " << data << "\n";
        } else
            std::cerr << "[AC Error] Not connected to DS.\n";
    }
}

void handleCommand(Context &ctx, ClientConnectionHandler *handler, const std::unique_ptr<Command> &command,
                   const int fd) {
    if (!command) {
        std::cerr << "[AC Error] Received invalid command from server.\n";
        return;
    }

    if (!ctx.codeState)
        std::cout << "[AC Log] Handling command ...\n";

    try {
        command->execute(ctx, fd);
    } catch (const std::exception &e) {
        std::cerr << "[AC Error] Command execution failed: " << e.what() << "\n";
    }
}

int main() {
    constexpr uint32_t AS_PORT = 27701;
    const std::string IP = "127.0.0.1";

    Context ctx{false, false, "0", nullptr, false};

    std::unique_ptr<ClientConnectionHandler> handler = nullptr;
    auto setupHandler = [&]() {
        try {
            handler = std::make_unique<ClientConnectionHandler>(EntityType::AUTH_CLIENT, IP, AS_PORT);
            handler->setCallback([&](const int fd, const std::unique_ptr<Command> &command) {
                handleCommand(ctx, handler.get(), command, fd);
            });
            ctx.client_handler = handler.get();
            ctx.isConnected = true;
        } catch (...) {
            ctx.isConnected = false;
            std::cerr << "[AC Error] Connection to AS Failed." << "\n";
        }
    };
    setupHandler();

    bool run = true;
    while (run) {
        if (ctx.isConnected && handler) {
            if (!handler->isRunning()) {
                std::cerr << "[AC Error] AS disconnected!\n";
                ctx.isConnected = false;
            } else
                handler->update();
        }

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (split(input)[0] == "exit") {
                    run = false;
                    continue;
                }
                if (split(input)[0] == "reconnect") {
                    setupHandler();
                    continue;
                }
                if (split(input)[0] == "clear" || split(input)[0] == "cls"
                        || split(input)[0] == "cl" || split(input)[0] == "clr") {
                    std::cout << "\033[2J\033[H" << std::flush;
                    continue;
                }
                handleUserInput(ctx, handler.get(), input);
            }
        }
    }
}
