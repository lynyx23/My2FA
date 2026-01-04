#ifndef MY2FA_SERVERCONTEXT_HPP
#define MY2FA_SERVERCONTEXT_HPP

#pragma once

class SessionManager;
class AuthManager;
class ServerConnectionHandler;

struct ServerContext {
    SessionManager &session_manager;
    AuthManager *auth_manager;
    ServerConnectionHandler *server_handler;
};

#endif //MY2FA_SERVERCONTEXT_HPP
