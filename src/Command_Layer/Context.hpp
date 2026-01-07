#ifndef MY2FA_SERVERCONTEXT_HPP
#define MY2FA_SERVERCONTEXT_HPP

#pragma once
#ifdef SERVER_SIDE
class SessionManager;
class AuthManager;
class ServerConnectionHandler;

struct Context {
    SessionManager &session_manager;
    AuthManager *auth_manager;
    ServerConnectionHandler &server_handler;
};
#else
// Client definition
#include <string>

struct Context {
    bool isLogged;
    std::string uuid;
};

#endif
#endif //MY2FA_SERVERCONTEXT_HPP
