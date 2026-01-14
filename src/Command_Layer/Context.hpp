#ifndef MY2FA_CONTEXT_HPP
#define MY2FA_CONTEXT_HPP

#pragma once
#include <string>
#ifdef A_SERVER
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
#endif
// Dummy Server definition
#ifdef D_SERVER
class SessionManager;
class AuthManager;
class ServerConnectionHandler;
class ClientConnectionHandler;

struct Context {
    SessionManager &session_manager;
    AuthManager *auth_manager;
    ServerConnectionHandler &server_handler;
    ClientConnectionHandler *client_handler;
    std::string app_id;
};
#endif
#if defined(A_CLIENT) || defined(D_CLIENT)
// Client definition
#include "Connection_Layer/ClientConnectionHandler.hpp"

struct Context {
    bool isLogged;
    bool isConnected;
    std::string username;
    ClientConnectionHandler *client_handler;
#ifdef A_CLIENT
    bool codeState;
    std::map<std::string, std::string> codes;
    time_t timeExpiration;
#endif
};

#endif
#endif //MY2FA_CONTEXT_HPP
