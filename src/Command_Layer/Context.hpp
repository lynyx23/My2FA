#ifndef MY2FA_CONTEXT_HPP
#define MY2FA_CONTEXT_HPP

#pragma once
#include <string>
#ifdef SERVER_SIDE
class SessionManager;
class AuthManager;
class ServerConnectionHandler;
class TOTPManager;

struct Context {
    SessionManager &session_manager;
    AuthManager *auth_manager;
    ServerConnectionHandler &server_handler;
    TOTPManager *totp_manager;
};
#else
// Client definition
#include <string>

struct Context {
    bool isLogged;
    std::string uuid;
    bool codeState;
    std::string code;
    time_t timeExpiration;
};

#endif
#endif //MY2FA_CONTEXT_HPP
