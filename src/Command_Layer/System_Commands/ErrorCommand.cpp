#include "ErrorCommand.hpp"

#include <iostream>
#include <sstream>
#include <utility>

ErrorCommand::ErrorCommand(const int errCode, std::string message)
        : m_code(errCode), m_msg(std::move(message)) {
}

std::string ErrorCommand::serialize() const  {
    std::ostringstream ss;
    ss << static_cast<int>(CommandType::ERR) << DELIMITER << m_code << DELIMITER << m_msg;
    return ss.str();
}

void ErrorCommand::execute(Context &ctx, const int fd) {
    std::cerr << "[Err] Error " << m_code << ": " << m_msg << "\n";
}

CommandType ErrorCommand::getType() const  {
    return CommandType::ERR;
}

int ErrorCommand::getCode() const {
    return m_code;
}

std::string ErrorCommand::getMessage() const {
    return m_msg;
}
