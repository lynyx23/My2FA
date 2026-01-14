#include "CodeResponseCommand.hpp"
#include <ctime>
#include <sstream>
#include <utility>
#ifdef A_CLIENT
#include <ctime>
#include "Command_Layer/Context.hpp"
#endif
CodeResponseCommand::CodeResponseCommand (const uint32_t remaining_time, std::string payload)
    : m_remaining_time(remaining_time), m_payload(std::move(payload)) {}

std::string CodeResponseCommand::serialize() const {
    std::stringstream ss;
    ss << static_cast<int>(CommandType::CODE_RESP) << DELIMITER << m_remaining_time << DELIMITER << m_payload;
    return ss.str();
}

void CodeResponseCommand::execute(Context &ctx, int client_fd) {
#ifdef A_CLIENT
    ctx.timeExpiration = std::time(nullptr) + m_remaining_time;
    ctx.codes.clear();
    std::stringstream ss(m_payload);
    std::string pair;
    while (std::getline(ss, pair, '|')) {
        const uint32_t separator = pair.find(':');
        ctx.codes[pair.substr(0, separator)] = pair.substr(separator + 1);
    }
#endif
};

CommandType CodeResponseCommand::getType() const {
    return CommandType::CODE_RESP;
}