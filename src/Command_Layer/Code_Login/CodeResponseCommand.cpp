#include "CodeResponseCommand.hpp"

#include <ctime>
#include <sstream>
#ifdef SERVER_SIDE
#else
#include <ctime>
#include "Command_Layer/Context.hpp"
#endif
CodeResponseCommand::CodeResponseCommand (const std::string &code, const uint32_t remaining_time)
    : m_code(code), m_remaining_time(remaining_time) {}

std::string CodeResponseCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::CODE_RESP) << DELIMITER << m_code
        << DELIMITER << m_remaining_time;
    return ss.str();
}

//TODO implement code receiving for clients
void CodeResponseCommand::execute(Context &ctx, int client_fd) {
#ifdef SERVER_SIDE
#else
    ctx.code = m_code;
    ctx.timeExpiration = std::time(nullptr) + m_remaining_time;
#endif
};

CommandType CodeResponseCommand::getType() const {
    return CommandType::CODE_RESP;
}

std::string CodeResponseCommand::getCode() const {
    return m_code;
}

uint32_t CodeResponseCommand::getRemainingTime() const {
    return m_remaining_time;
}