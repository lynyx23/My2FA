#include "AuthManager.hpp"
#include <iostream>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

struct OpenSSLFree {
    void operator()(void* ptr) const {
        EVP_MD_CTX_free(static_cast<EVP_MD_CTX *>(ptr));
    }
};

AuthManager::AuthManager(const std::string& server_type)
    : m_db(server_type + ".db", SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE) {
    init();
}

void AuthManager::init() {
    try {
        m_db.exec("CREATE TABLE IF NOT EXISTS users ("
                  "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                  "username TEXT UNIQUE NOT NULL,"
                  "pass_hash TEXT NOT NULL,"
                  "salt TEXT NOT NULL,"
                  "uuid TEXT UNIQUE,"
                  "totp_secret TEXT NOT NULL DEFAULT '')");
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Init error: " << e.what() << "\n";
    }
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

std::string AuthManager::getUUID(const std::string &username) const {
    SQLite::Statement query(m_db, "SELECT uuid FROM users WHERE username = ?");
    query.bind(1, username);
    try {
        query.executeStep();
    } catch (std::exception& e) {
        std::cerr << "[AM Error] UUID lookup failed: " << e.what() << "\n";
    }
    return query.getColumn(0);
}

std::string AuthManager::getSecret(const std::string &uuid) const {
    SQLite::Statement query(m_db, "SELECT totp_secret FROM users WHERE uuid = ?");
    query.bind(1, uuid);
    try {
        query.executeStep();
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Secret lookup failed: " << e.what() << "\n";
    }
    return query.getColumn(0);
}

void AuthManager::setSecret(const std::string &uuid, const std::string &secret) {
    SQLite::Statement query(m_db, "UPDATE users SET totp_secret = ? WHERE uuid = ?");
    query.bind(1, secret);
    query.bind(2, uuid);
    try {
        query.exec();
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Secret update failed: " << e.what() << "\n";
    }
}

//TODO username and password regex checking
bool AuthManager::loginUser(const std::string& username, const std::string& password) const {
    SQLite::Statement query(m_db, "SELECT username, pass_hash, salt, uuid FROM users WHERE username = ?");
    query.bind(1, username);

    if (query.executeStep()) {
        const std::string hash = query.getColumn(1);
        const std::string salt = query.getColumn(2);
        const std::string uuid = query.getColumn(3);

        if (m_hashPassword(password, salt) == hash) return true;
    }

    return false;
}

std::string AuthManager::m_generateUUID(const std::string& username) {
    // this uses a modified version of UUIDv4, using the username as a unique factor
    // the random string and username are hashed using SHAKE128 from OpenSSL
    unsigned char random[16];
    RAND_bytes(random, 16);

    unsigned char hash[EVP_MAX_MD_SIZE];

    std::unique_ptr<EVP_MD_CTX, OpenSSLFree> ctx(EVP_MD_CTX_new());
    if (ctx) {
        try {
            EVP_DigestInit_ex(ctx.get(), EVP_shake128(), nullptr);
            EVP_DigestUpdate(ctx.get(), random, sizeof(random));
            EVP_DigestUpdate(ctx.get(), username.c_str(), username.size());
            EVP_DigestFinalXOF(ctx.get(), hash, 16);
        } catch (std::exception& e) {
            std::cerr << "[AM Error] UUID generation error: " << e.what() << "\n";
        }
    }

    hash[6] = (hash[6] & 0x0f) | 0x40; // sets the version bit to 4
    hash[8] = (hash[8] & 0x3f) | 0x80; // sets the variant bit to 1

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; i++) {
        ss << std::setw(2) << static_cast<int>(hash[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-";
    }
    return ss.str();
}

void AuthManager::testAddUser(const std::string &username, const std::string &password, const std::string &secret) {
    SQLite::Statement query(m_db, "INSERT INTO users (username, pass_hash, salt, uuid, totp_secret) VALUES (?, ?, ?, ?, ?)");

    query.bind(1, username);
    const std::string salt = m_generateSalt();
    query.bind(2, m_hashPassword(password, salt));
    query.bind(3, salt);
    query.bind(4, m_generateUUID(username));
    query.bind(5, secret);

    std::cout << "[AM Log] User added: " << username << "\n";
    try {
        query.exec();
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Adding user failed: " << e.what() << "\n";
    }
}

void AuthManager::testRemoveUser(const std::string &username) {
    SQLite::Statement query(m_db, "DELETE FROM users WHERE username = ?");
    query.bind(1, username);

    try {
        query.exec();
        std::cout << "[AM Log] User removed: " << username << "\n";
    } catch (std::exception& e) {
        std::cerr << "[AM Error] Removing user failed: " << e.what() << "\n";
    }
}

void AuthManager::show() const {
    SQLite::Statement query(m_db, "SELECT * FROM users");
    while (query.executeStep()) {
        std::cout << "username = " << query.getColumn(1) << "\n" << "pass_hash = " << query.getColumn(2) << "\n"
            << "salt = " << query.getColumn(3) << "\n" << "uuid = " << query.getColumn(4) << "\n"
            << "totp_secret = " << query.getColumn(5) << "\n";
    }
}

void AuthManager::placeholder() const {
    SQLite::Statement query(m_db, "UPDATE users SET totp_secret = ? WHERE username = ?");
    query.bind(1, "HXDMMJECJJWSRB3HWIZR4IFUGFTMXBOZ");
    query.bind(2, "admin");
    query.exec();
}

