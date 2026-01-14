#include "GenericResponseCommand.hpp"
#include <sstream>

#include "Connection_Layer/ServerConnectionHandler.hpp"
#include "Session_Manager/SessionManager.hpp"
#if defined(A_CLIENT) || defined(D_CLIENT)
#include "Command_Layer/Context.hpp"
#elif defined(D_SERVER)
#include "Command_Layer/Context.hpp"
#endif

GenericResponseCommand::GenericResponseCommand(const CommandType type, const bool resp,
        std::string message, std::string extra):
    m_type(type), m_resp(resp), m_msg(std::move(message)), m_extra(std::move(extra)) {}

std::string GenericResponseCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::RESP) << DELIMITER << static_cast<int>(m_type)
        << DELIMITER << m_resp << DELIMITER << m_msg << DELIMITER << m_extra;
    return ss.str();
}

void GenericResponseCommand::execute(Context &ctx, int fd) {
    //std::cout << "[DEBUG] type " << m_type << " | resp " << m_resp << " | msg " << m_msg << " | extra " << m_extra << "\n" ;
    switch (m_type) {
        case CommandType::LOGIN_RESP:
#if defined(A_CLIENT) || defined(D_CLIENT)
            if (m_resp) {
                ctx.isLogged = true;
                ctx.username = m_extra;
                std::cout << "[Client] Login successful for user " << m_extra << "!\n";
            } else {
                ctx.isLogged = false;
                std::cerr << "[Client] Login failed: " << m_extra << "\n";
            }
#endif
            break;
        case CommandType::REGISTER_RESP:
#if defined(A_CLIENT) || defined(D_CLIENT)
            if (m_resp) {
                std::cout << "[Client] Registration successful for user " << m_extra << "!\n";
            }
            else std::cerr << "[Client] Registration failed for user " << m_extra << "!\n";
#endif
            break;
        case CommandType::PAIR_RESP:
#ifdef D_SERVER
            ctx.server_handler.sendCommand(ctx.session_manager.getIDFromUsername(m_extra),
                std::make_unique<GenericResponseCommand>(CommandType::PAIR_RESP, m_resp, m_msg, m_extra));
#elif defined(D_CLIENT)
            if (m_resp && !m_msg.empty() && !m_extra.empty()) {
                std::cout << "[2FA Pairing] Pairing request successful for user " << m_extra << "!\n"
                          << "[2FA Pairing] Use token \033[32m" << m_msg << "\033[0m on Auth Client!\n";
            }
            else if (m_resp && m_msg.empty() && !m_extra.empty()) {
                std::cout << "[2FA Pairing] Pairing completed for user " << m_extra << "!\n";
            }
            else std::cerr << "[2FA Pairing] A pairing request already exists for " << m_extra << "!\n";
#endif
            break;
        case::CommandType::CODE_CHK_RESP:
#ifdef D_CLIENT
            if (m_resp) std::cout << "\033[118m[2FA Check] 2FA Code Login Successful !\033[0m\n";
            else std::cerr << "[2FA Check] 2FA Code Login Failed!\n";
#endif
            break;
        default:
            std::cerr << "[Client] Unknown response type!\n";
            break;
    }
}

std::string GenericResponseCommand::getMessage() const { return m_msg; }

CommandType GenericResponseCommand::getType() const { return m_type; }

bool GenericResponseCommand::getResponse() const { return m_resp; }
