#ifndef MY2FA_COMMANDTYPES_HPP
#define MY2FA_COMMANDTYPES_HPP

enum class CommandType {
    // System Commands
    CONN = 1,
    PING = 2,
    ERR = 3,

    // Credential Login Commands
    LOGIN_REQ = 11,
    LOGIN_RESP = 12,

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

constexpr char DELIMITER = ';';

#endif //MY2FA_COMMANDTYPES_HPP