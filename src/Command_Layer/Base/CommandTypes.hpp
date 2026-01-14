#ifndef MY2FA_COMMANDTYPES_HPP
#define MY2FA_COMMANDTYPES_HPP

#pragma once
#include <cstdint>
#include <iostream>

enum class CommandType : uint8_t {
    // System Commands
    CONN = 1,
    PING = 2,
    ERR = 3,
    RESP = 4,
    PAIR_REQ = 5,
    UNKNOWN = 6,

    // Credential Login Commands
    CRED_REQ = 11,
    LOGIN_REQ = 12,
    LOGOUT_REQ = 13,
    REGISTER_REQ = 14,

    // Notification Login Commands
    REQ_NOTIF = 21,
    SEND_NOTIF = 22,

    // Code Login Commands
    REQ_CODE_CLIENT = 31,
    CODE_RESP = 32,
    VALIDATE_CODE_CLIENT = 33,
    VALIDATE_CODE_SERVER = 34,
    VALIDATE_RESP_SERVER = 35,
    VALIDATE_RESP_CLIENT = 36,

    // Response Types
    LOGIN_RESP = 41,
    REGISTER_RESP = 42,
    PAIR_RESP = 43,
    CODE_CHK_RESP = 44,
    NOTIF_RESP = 45,
    NOTIF_LOGIN_RESP = 46,


    // Others
    EXIT_SCS = 51
};

inline std::ostream& operator<<(std::ostream& os, const CommandType& type) {
    switch (type) {
        case CommandType::CONN: return os << "CONN";
        case CommandType::PING: return os << "PING";
        case CommandType::ERR: return os << "ERR";
        case CommandType::RESP: return os << "RESP";
        case CommandType::CRED_REQ: return os << "CRED_REQ";
        case CommandType::LOGIN_REQ: return os << "LOGIN_REQ";
        case CommandType::LOGIN_RESP: return os << "LOGIN_RESP";
        case CommandType::LOGOUT_REQ: return os << "LOGOUT_REQ";
        case CommandType::REGISTER_REQ: return os << "REGISTER_REQ";
        case CommandType::REGISTER_RESP: return os << "REGISTER_RESP";
        case CommandType::SEND_NOTIF: return os << "SEND_NOTIF";
        case CommandType::REQ_CODE_CLIENT: return os << "REQ_CODE_CLIENT";
        case CommandType::CODE_RESP: return os << "CODE_RESP";
        case CommandType::VALIDATE_CODE_CLIENT: return os << "VALIDATE_CODE_CLIENT";
        case CommandType::VALIDATE_CODE_SERVER: return os << "VALIDATE_CODE_SERVER";
        case CommandType::VALIDATE_RESP_SERVER: return os << "VALIDATE_RESP_SERVER";
        case CommandType::EXIT_SCS: return os << "EXIT_SCS";
        default: return os << "UNKNOWN";
    }
}

constexpr char DELIMITER = ';';

#endif //MY2FA_COMMANDTYPES_HPP
