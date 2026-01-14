#include "PairCommand.hpp"
#include <sstream>
#include "GenericResponseCommand.hpp"
#ifdef D_SERVER
#include "Command_Layer/Context.hpp"
#include "Connection_Layer/ClientConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#elif defined(A_SERVER)
#include "Command_Layer/Context.hpp"
#include "Database_Layer/Database.hpp"
#include "Session_Manager/SessionManager.hpp"
#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Auth_Layer/AuthManager.hpp"
#endif

PairCommand::PairCommand(std::string d_username):
    m_d_username(std::move(d_username)) {}

std::string PairCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::PAIR_REQ);
#ifdef D_SERVER
    ss << DELIMITER << m_d_username;
#endif
    return ss.str();
}

void PairCommand::execute(Context &ctx, const int fd) {
#ifdef D_SERVER
    //std::cout << "[DEBUG] propagating to AS " << m_d_username << "\n";
    ctx.client_handler->sendCommand(
        std::make_unique<PairCommand>(ctx.session_manager.getIdentity(fd)));
#elif defined(A_SERVER)
    std::string token = ctx.auth_manager->startPairing(m_d_username, ctx.session_manager.getIdentity(fd));
    //std::cout << "[DEBUG] token = " << token << "\n";
    bool resp = true;
    if (token.empty()) resp = false;
    ctx.server_handler.sendCommand(fd,
            std::make_unique<GenericResponseCommand>(CommandType::PAIR_RESP, resp, token, m_d_username));
#endif
}

CommandType PairCommand::getType() const {
    return CommandType::PAIR_REQ;
}

std::string PairCommand::getCode() const {
    return m_code;
}

std::string PairCommand::getD_username() const {
    return m_d_username;
}

