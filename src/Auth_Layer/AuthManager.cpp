#include "AuthManager.hpp"
#include <iomanip>
#include <iostream>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>
#include "Database_Layer/Database.hpp"
#include "Session_Manager/SessionManager.hpp"

struct OpenSSLFree {
    void operator()(void* ptr) const {
        EVP_MD_CTX_free(static_cast<EVP_MD_CTX *>(ptr));
    }
};

AuthManager::AuthManager(const std::string &server_type):
    m_server_type(server_type) {
    Database::init(server_type);
}

std::string AuthManager::m_hashPassword(const std::string& password, const std::string& salt) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length = 0;

    std::unique_ptr<EVP_MD_CTX, OpenSSLFree> ctx(EVP_MD_CTX_new());
    if (ctx) {
        try {
            EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr);
            EVP_DigestUpdate(ctx.get(), password.c_str(), password.size());
            EVP_DigestUpdate(ctx.get(), salt.c_str(), salt.size());
            EVP_DigestFinal_ex(ctx.get(), hash, &hash_length);
        } catch (std::exception& e) {
            std::cerr << "[AM Error] Hashing error: " << e.what() << "\n";
        }
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i=0; i < hash_length; i++) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string AuthManager::m_generateSecret() {
    constexpr char base32[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    constexpr uint8_t length = 32;

    std::vector<unsigned char> entropy(length);
    RAND_bytes(entropy.data(), entropy.size());

    std::string secret;
    for (const unsigned char c : entropy)
        secret += base32[c % length];
    return secret;
}

std::string AuthManager::m_generateSalt() {
    unsigned char salt[16];
    try {
        RAND_bytes(salt, 16);
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Salt generation error: " << e.what() << "\n";
    }
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const unsigned char i : salt) {
        ss << std::setw(2) << static_cast<int>(i);
    }
    return ss.str();
}

std::string AuthManager::m_generateToken() {
    constexpr char dict[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    constexpr uint8_t entropy_len = 8;
    constexpr uint8_t dict_len = sizeof(dict) - 1;

    std::vector<unsigned char> entropy(entropy_len);
    RAND_bytes(entropy.data(), entropy_len);

    std::string token;
    for (const unsigned char c : entropy)
        token += dict[c % dict_len];
    return token;
}

std::string AuthManager::m_generateReqID() const {
    constexpr char dict[] = "ABCDEFGHIJKLMOPRQSTUVWXYZ0123456789";
    constexpr uint8_t entropy_len = 5;
    constexpr uint8_t dict_len = sizeof(dict) - 1;

    std::vector<unsigned char> entropy(entropy_len);
    RAND_bytes(entropy.data(), entropy_len);

    std::string reqID;
    for (const unsigned char c : entropy)
        reqID += dict[c % dict_len];
    return reqID;
}

// TODO username and password regex checking
bool AuthManager::loginUser(const std::string& username, const std::string& password) {

    if (const std::optional<Database::UserDTO> result = Database::getUser(username);
        result.has_value()) {
        if (m_hashPassword(password, result->salt) == result->pass_hash)
            return true;
    }
    return false;
}

bool AuthManager::registerUser(const std::string &username, const std::string &password) {
    const std::string salt = m_generateSalt();
    const std::string pass_hash = m_hashPassword(password, salt);
#ifdef A_SERVER
    const Database::UserDTO user{username, pass_hash, salt, m_generateSecret()};
#else
    const Database::UserDTO user{username, pass_hash, salt};
#endif
    if (Database::createUser(user)) return true;
    return false;
}

std::string AuthManager::startPairing(const std::string &d_username, const std::string &app_id) {
    for (auto &pair: m_pending_pairings | std::views::values) {
        if (pair.d_username == d_username && pair.app_id == app_id)
            return "";
    }

    const std::string secret = m_generateSecret();
    const std::string token = m_generateToken();

    PendingPairing pairing;
    pairing.d_username = d_username;
    pairing.app_id = app_id;
    pairing.secret = secret;

    m_pending_pairings[token] = pairing;
    std::cout << "[AM Log] Generated token " << token << " for " << d_username << "\n";
    return token;
}

std::optional<std::pair<std::string, std::string>> AuthManager::finishPairing(const std::string &a_username, const std::string &token) {
    auto it = m_pending_pairings.find(token);
    if (it == m_pending_pairings.end()) {
        std::cerr << "[AM Error] Invalid pairing token: " << token << "!\n";
        return std::nullopt;
    }
    if (Database::pairUser(a_username, it->second.d_username, it->second.app_id, it->second.secret)) {
        auto pair = std::make_pair(it->second.app_id, it->second.secret);
        m_pending_pairings.erase(it);
        return pair;
    }
    return std::nullopt;
}

int AuthManager::startNotification(const std::string &username, const std::string &app_id, std::string &reqID, const int &ds_fd, SessionManager &session_manager) {
    const auto a_user_resp = Database::getA_username(username, app_id);
    if (!a_user_resp.has_value()) {
        std::cerr << "[AM Error] User not found!\n";
        return -1;
    }
    const int ac_fd = session_manager.getIDFromUsername(a_user_resp.value());
    if (ac_fd <= 0) {
        std::cerr << "[AM Error] User not logged in!\n";
        return -1;
    }

    PendingNotification notification;
    notification.ds_fd = ds_fd;
    notification.app_id = app_id;
    notification.d_username = username;
    reqID = m_generateReqID();
    m_pending_notifications[reqID] = notification;

    std::cout << "[AM Log] Notification " << reqID << " created for " << a_user_resp.value() << "\n";
    return ac_fd;
}

int AuthManager::finishNotification(const std::string &reqID, std::string &d_username) {
    if (const auto it = m_pending_notifications.find(reqID); it != m_pending_notifications.end()) {
        const int ds_fd = it->second.ds_fd;
        d_username = it->second.d_username;
        m_pending_notifications.erase(it);
        return ds_fd;
    }
    std::cerr << "[AM Error] Invalid notification ID: " << reqID << "!\n";
    return -1;
}

void AuthManager::show() {
    Database::show();
}

// std::string AuthManager::m_generateUUID(const std::string& username) {
//     // this uses a modified version of UUIDv4, using the username as a unique factor
//     // the random string and username are hashed using SHAKE128 from OpenSSL
//     unsigned char random[16];
//     RAND_bytes(random, 16);
//
//     unsigned char hash[EVP_MAX_MD_SIZE];
//
//     std::unique_ptr<EVP_MD_CTX, OpenSSLFree> ctx(EVP_MD_CTX_new());
//     if (ctx) {
//         try {
//             EVP_DigestInit_ex(ctx.get(), EVP_shake128(), nullptr);
//             EVP_DigestUpdate(ctx.get(), random, sizeof(random));
//             EVP_DigestUpdate(ctx.get(), username.c_str(), username.size());
//             EVP_DigestFinalXOF(ctx.get(), hash, 16);
//         } catch (std::exception& e) {
//             std::cerr << "[AM Error] UUID generation error: " << e.what() << "\n";
//         }
//     }
//
//     hash[6] = (hash[6] & 0x0f) | 0x40; // sets the version bit to 4
//     hash[8] = (hash[8] & 0x3f) | 0x80; // sets the variant bit to 1
//
//     std::stringstream ss;
//     ss << std::hex << std::setfill('0');
//     for (int i = 0; i < 16; i++) {
//         ss << std::setw(2) << static_cast<int>(hash[i]);
//         if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-";
//     }
//     return ss.str();
// }
//TODO do SSL encryption on data sent
