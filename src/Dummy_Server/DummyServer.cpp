#include <sstream>
#include <sys/types.h>
#include <sys/time.h>
#include <csignal>

#include "Auth_Layer/AuthManager.hpp"
#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Context.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/System_Commands/SystemCommands.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"

void handleCommand(const std::unique_ptr<Command> &command, const int sender_fd, Context &ctx) {
    if (!command) {
        std::cerr << "[DS Error] Received invalid command!\n";
        return;
    }
    try {
        command->execute(ctx, sender_fd);
    } catch (std::exception &e) {
        std::cerr << "[DS Error] Command execution failed: " << e.what() << "\n";
    }
}

//TODO DONT FORGET TO MAKE IT SO THAT A USER CAN HAVE MULTIPLE APP SERVERS LOGGED

void handleUserInput(Context &ctx, const std::string &input) {
    auto args = split(input);
    if (args.empty()) return;

    std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << "  clients                                     : List all active clients.\n"
                  << "  reconnect                                   : Reconnect to AS\n"
                  << "  err <code> <msg>                            : Send Error to AS\n"
                  << "  req_notif_server <uuid> <appid>             : Send Notification Request to AS\n"
                  << "  notif_resp_server <resp> <uuid>             : Send Notification Result to DC\n"
                  << "  validate_code_server <code> <uuid> <appid>  : Send Code Verify Request to AS\n"
                  << "  validate_resp_client <resp> <uuid>          : Send Code Result to DC\n"
                  << "  exit                                        : Exit\n";
        return;
    } else if (args[0] == "reconnect") return;
    else if (args[0] == "clients") {
        ctx.session_manager.displayConnections();
    } else if (args[0] == "err") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: err <code> <msg>\n";
            return;
        }
        command = std::make_unique<ErrorCommand>(std::stoi(args[1]), args[2]);
    } else if (args[0] == "req_notif_server") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: req_notif_server <uuid> <appid>\n";
            return;
        }
        command = std::make_unique<RequestNotificationServerCommand>(args[1], std::stoi(args[2]));
    } else if (args[0] == "notif_resp_server") {
        if (args.size() < 3) {
            std::cout << "[DS Error] Usage: notif_resp_server <1/0> <uuid>\n";
            return;
        }
        bool resp = (args[1] == "1" || args[1] == "true");
        command = std::make_unique<NotificationResponseServerCommand>(resp, args[2]);
    } else if (args[0] == "validate_code_server") {
        if (args.size() < 4) {
            std::cout << "[DS Error] Usage: validate_code_server <code> <username> <appid>\n";
            return;
        }
        command = std::make_unique<ValidateCodeServerCommand>(args[1], args[2], args[3]);
    } else {
        std::cout << "[DS Error] Unknown command! Type help.\n";
        return;
    }

    if (command) {
        const std::string data = command->serialize();

        switch (command->getType()) {
            case CommandType::CONN:
            case CommandType::REQ_NOTIF_SERVER:
            case CommandType::VALIDATE_CODE_SERVER:
                // try catch doesn't work
                if (ctx.client_handler && ctx.client_handler->isRunning()) {
                    ctx.client_handler->sendCommand(command);
                    std::cout << "[DS -> AS] Sent: " << data << "\n";
                } else {
                    std::cerr << "[DS Error] Not connected to Auth Server.\n";
                }
                break;
            default:
                try {
                    ctx.server_handler.broadcastCommand(command);
                } catch (std::exception &e) {
                    std::cerr << "[DS Error] No routing rule for Command: "
                        << command->getType() << "!\n";
                }
                break;
        }
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
    uint32_t DS_PORT = 27702;
    try {
        DS_PORT = std::stoi(argv[1]);
    } catch (std::exception &e) {
        std::cerr << "[DS Error] Invalid port: " << e.what() << " | " << argv[1] << "\n";
    }
    constexpr uint32_t AS_PORT = 27701;  // Auth Server Port
    const std::string app_id = "app1";
    const std::string IP = "127.0.0.1";

    signal(SIGPIPE, SIG_IGN); // makes sure the server doesn't crash if it interacts with a dead socket

    ServerConnectionHandler ds_handler(DS_PORT);
    SessionManager session_manager;
    AuthManager auth_manager("ds_" + app_id);

    Context ctx{session_manager, &auth_manager, ds_handler, nullptr, app_id};

    ds_handler.setCommandCallback([&](const int client_fd, const std::unique_ptr<Command> &command) {
        std::cout << "[DS Log] Handling command from Client " << client_fd << "\n";
        handleCommand(command, client_fd, ctx);
    });
    ds_handler.setConnectCallback([&](const int client_fd) {
        std::cout << "[DS Log] New client connected: " << client_fd << "\n";
        session_manager.addSession(client_fd);
    });
    ds_handler.setDisconnectCallback([&](const int client_fd) {
        std::cout << "[DS Log] Client disconnected: " << client_fd << "\n";
        session_manager.removeSession(client_fd);
    });

    bool as_connected;
    std::unique_ptr<ClientConnectionHandler> as_handler = nullptr;
    auto setupASHandler = [&]() {
        try {
            as_handler = std::make_unique<ClientConnectionHandler>(EntityType::DUMMY_SERVER, IP, AS_PORT, app_id);
            as_handler->setCallback([&](const int fd, const std::unique_ptr<Command> &command) {
                std::cout << "[DS Log] Handling command from AS ...\n";
                handleCommand(command, fd, ctx);
            });
            ctx.client_handler = as_handler.get();
            as_connected = true;
        } catch (...) {
            as_connected = false;
            std::cerr << "[DS Error] Could not connect to AS.\n";
        }
    };
    setupASHandler();

    bool run = true;
    while (run) {
        ds_handler.update();
        if (as_connected && as_handler) {
            if (!as_handler->isRunning()) {
                std::cerr << "[DS Error] Auth Server disconnected.\n";
                as_connected = false;
            } else as_handler->update();
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
                    setupASHandler();
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
