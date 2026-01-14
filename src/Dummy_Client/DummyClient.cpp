#include <sstream>

#include "Command_Layer/Base/EntityType.hpp"
#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Context.hpp"
#include "Command_Layer/Credential_Login/CredentialLoginCommands.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/System_Commands/PairCommand.hpp"
#include "Command_Layer/System_Commands/SystemCommands.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"

void handleUserInput(Context &ctx, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << std::flush
                  << "  help                      : Show this menu\n"
                  << "  conn;<id>                 : Handshake (e.g. conn;dc_01)\n"
                  << "  login;<user>;<pass>       : Credential Login (e.g. login;user;pass)\n"
                  << "  code;<code>               : Enter 2FA Code (e.g. code;123456)\n"
                  << "  reconnect                 : Reconnect to DS\n"
                  << "  exit                      : Quit\n";
        return;
    } else if (args[0] == "reconnect") return;
    else if (args[0] == "conn") {
        if (args.size() != 2) {
            std::cout << "[DC Error] Usage: conn <id>\n";
            return;
        }
        command = std::make_unique<ConnectCommand>(EntityType::DUMMY_CLIENT);
    } else if (args[0] == "pair") {
        if (args.size() != 1) {
            std::cout << "[DC Error] Usage: pair\n";
            return;
        }
        command = std::make_unique<PairCommand>();
    } else if (args[0] == "login") {
        if (args.size() != 3) {
            std::cout << "[DC Error] Usage: login <user> <pass>\n";
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
    } else if (args[0] == "code") {
        if (args.size() != 2) {
            std::cout << "[DC Error] Usage: code;<code>\n";
            return;
        }
        command = std::make_unique<ValidateCodeClientCommand>(args[1]);
    } else if (args[0] == "notif") {
        if (args.size() != 2) {
            std::cout << "[DC Error] Usage: notif;<user>\n";
            return;
        }
        command = std::make_unique<RequestNotificationCommand>(args[1]);
    } else {
        std::cout << "[DC Error] Unknown command! Type help.\n";
        return;
    }

    if (command) {
        const std::string data = command->serialize();
        if (ctx.client_handler && ctx.client_handler->isRunning()) {
            ctx.client_handler->sendCommand(command);
            std::cout << "[DC -> DS] Sent: " << data << "\n";
        } else std::cerr << "[DC Error] Not connected to DS.\n";
    }
}

void handleCommand(const std::unique_ptr<Command> &command, Context &ctx, const int fd) {
    if (!command) {
        std::cerr << "[DC Error] Received invalid command from server.\n";
        return;
    }
    try {
        command->execute(ctx, fd);
    } catch (const std::exception &e) {
        std::cerr << "[DC Error] Command execution failed: " << e.what() << "\n";
    }
}

bool checkConsoleInput() {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    timeval timeout{0, 0};
    return select(STDIN_FILENO + 1, &read_set, nullptr, nullptr, &timeout) > 0;
}

int main(int argc, char *argv[]) {
    std::string IP = "127.0.0.1";
    int DS_PORT = 27702;
    if (argc == 2) {
        IP = argv[1];
    } else if (argc == 3) {
        IP = argv[1];
        try {
            DS_PORT = std::stoi(argv[2]);
        } catch (std::exception &e) {
            std::cerr << "[AC Error] Invalid port: " << e.what() << " | " << argv[2] << "\n";
            return 1;
        }
    }

    std::unique_ptr<ClientConnectionHandler> handler = nullptr;

    Context ctx{false, false, "", handler.get()};

    auto setupHandler = [&]() {
        try {
            handler = std::make_unique<ClientConnectionHandler>(EntityType::DUMMY_CLIENT, IP, DS_PORT);
            handler->setCallback([&](const int fd, const std::unique_ptr<Command> &command) {
                std::cout << "[DC Log] Handling command ...\n";
                handleCommand(command, ctx, fd);
            });
            ctx.client_handler = handler.get();
            ctx.isConnected = true;
        } catch (...) {
            ctx.isConnected = false;
            std::cerr << "[DC Error] Connection to DS Failed." << "\n";
        }
    };
    setupHandler();

    bool run = true;
    while (run) {
        if (ctx.isConnected && handler) {
            if (!handler->isRunning()) {
                std::cerr << "[DC Error] DS disconnected!\n";
                ctx.isConnected = false;
            } else handler->update();
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
                handleUserInput(ctx, input);
            }
        }
    }
}
