//#ifdef A_SERVER
#include <sstream>
#include <csignal>
#include <sys/types.h>
#include <sys/time.h>

#include "Command_Layer/CommandFactory.hpp"
#include "Command_Layer/Notification_Login/NotificationLoginCommands.hpp"
#include "Command_Layer/Code_Login/CodeLoginCommands.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Auth_Layer/AuthManager.hpp"
#include "Command_Layer/Context.hpp"
#include "TOTP_Layer/TOTPManager.hpp"

void handleCommand(const std::unique_ptr<Command> &command, const int client_fd, Context &ctx) {
    if (!command) {
        std::cerr << "[AS Error] Received invalid command from client " << client_fd << "\n";
        return;
    }
    try {
        command->execute(ctx, client_fd);
    } catch (const std::exception &e) {
        std::cerr << "[AS Error] Command " << "(" << command->getType() << ") execution failed: " << e.what() << "\n";
    }
}

void handleUserInput(ServerConnectionHandler &handler, const std::string &input, SessionManager &session_manager) {
    auto args = split(input);
    if (args.empty()) return;

    //std::unique_ptr<Command> command = nullptr;

    if (args[0] == "help") {
        std::cout << std::flush
                  << "  help       : Shows this menu\n"
                  << "  clients    : List all active client file descriptors.\n"
                  << "  db         : Prints all data in DB.\n"
                  << "  clear      : Clears screen (aliases: cl, cls, clr)"
                  << "  exit       : Shut down the server.\n";
        return;
    } if (args[0] == "clients") {
        session_manager.displayConnections();
    } else {
        std::cerr << "[AS Error] Unknown command. Type 'help'.\n";
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
    int PORT = 27701;
    if (argc == 2) {
        try {
            PORT = std::stoi(argv[1]);
        } catch (std::exception &e) {
            std::cerr << "[AC Error] Invalid port: " << e.what() << " | " << argv[2] << "\n";
            return 1;
        }
    }

    signal(SIGPIPE, SIG_IGN); // avoid crashes from sending

    AuthManager auth_manager("as");
    SessionManager session_manager;
    ServerConnectionHandler handler(PORT);

    Context ctx{session_manager, &auth_manager, handler, nullptr};

    TOTPManager totp_manager(ctx);
    ctx.totp_manager = &totp_manager;
    totp_manager.start();

    handler.setCommandCallback([&](const int client_fd, const std::unique_ptr<Command> &command) {
        std::cout << "[AS Log] Handling command from Client: " << client_fd << " ("
            << ctx.session_manager.getEntityType(client_fd) << ")\n";
        handleCommand(command, client_fd, ctx);
    });
    handler.setConnectCallback([&](const int client_fd) {
        session_manager.addSession(client_fd);
        std::cout << "[AS Log] New client connected: " << client_fd << "\n";
    });
    handler.setDisconnectCallback([&](const int client_fd) {
        session_manager.removeSession(client_fd);
        std::cout << "[AS Log] Client disconnected: " << client_fd << "\n";
    });

    bool run = true;
    while (run) {
        handler.update();

        if (checkConsoleInput()) {
            std::string input;
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (split(input)[0] == "exit") {
                    run = false;
                    continue;
                }
                if (split(input)[0] == "clear" || split(input)[0] == "cls"
                        || split(input)[0] == "cl" || split(input)[0] == "clr") {
                    std::cout << "\033[2J\033[H" << std::flush;
                    continue;
                }
                if (split(input)[0] == "db") {
                    ctx.auth_manager->show();
                    continue;
                }
                handleUserInput(handler, input, session_manager);
            }
        }
    }
}
//#endif // A_SERVER