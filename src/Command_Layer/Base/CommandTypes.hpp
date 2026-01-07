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
    UNKNOWN = 4,

    // Credential Login Commands
    LOGIN_REQ = 11,
    LOGIN_RESP = 12,
    LOGOUT_REQ = 13,

    // Notification Login Commands
    REQ_NOTIF_CLIENT = 21,
    REQ_NOTIF_SERVER = 22,
    SEND_NOTIF = 23,
    NOTIF_RESP_CLIENT = 24,
    NOTIF_RESP_SERVER = 25,

    // Code Login Commands
    REQ_CODE_CLIENT = 31,
    CODE_RESP = 32,
    VALIDATE_CODE_CLIENT = 33,
    VALIDATE_CODE_SERVER = 34,
    VALIDATE_RESP_SERVER = 35,
    VALIDATE_RESP_CLIENT = 36
};

inline std::ostream& operator<<(std::ostream& os, const CommandType& type) {
    switch (type) {
        case CommandType::CONN: return os << "CONN";
        case CommandType::PING: return os << "PING";
        case CommandType::ERR: return os << "ERR";
        case CommandType::LOGIN_REQ: return os << "LOGIN_REQ";
        case CommandType::LOGIN_RESP: return os << "LOGIN_RESP";
        case CommandType::REQ_NOTIF_CLIENT: return os << "REQ_NOTIF_CLIENT";
        case CommandType::REQ_NOTIF_SERVER: return os << "REQ_NOTIF_SERVER";
        case CommandType::SEND_NOTIF: return os << "SEND_NOTIF";
        case CommandType::NOTIF_RESP_CLIENT: return os << "NOTIF_RESP_CLIENT";
        case CommandType::NOTIF_RESP_SERVER: return os << "NOTIF_RESP_SERVER";
        case CommandType::REQ_CODE_CLIENT: return os << "REQ_CODE_CLIENT";
        case CommandType::CODE_RESP: return os << "CODE_RESP";
        case CommandType::VALIDATE_CODE_CLIENT: return os << "VALIDATE_CODE_CLIENT";
        case CommandType::VALIDATE_CODE_SERVER: return os << "VALIDATE_CODE_SERVER";
        case CommandType::VALIDATE_RESP_SERVER: return os << "VALIDATE_RESP_SERVER";
        case CommandType::VALIDATE_RESP_CLIENT: return os << "VALIDATE_RESP_CLIENT";
        default: return os << "UNKNOWN";
    }
}

constexpr char DELIMITER = ';';

#endif //MY2FA_COMMANDTYPES_HPP
