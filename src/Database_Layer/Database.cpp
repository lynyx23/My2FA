#include "Database.hpp"
#include <filesystem>
#include <iostream>
#include <optional>
#include <map>
#include <SQLiteCpp/SQLiteCpp.h>

namespace Database {
    static std::unique_ptr<SQLite::Database> db = nullptr;

    void init(const std::string &server_type) {
        try {
            const std::filesystem::path db_path = std::filesystem::path(__FILE__).
                parent_path() / "dbs" / (server_type + ".db");
            //std::cout << "[DEBUG DB] path = " << db_path.string() << "\n";
            db = std::make_unique<SQLite::Database>(
            db_path.string(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

            db->exec("PRAGMA foreign_keys = ON");
#ifdef A_SERVER
            db->exec("CREATE TABLE IF NOT EXISTS users ("
                  "username TEXT PRIMARY KEY NOT NULL,"
                  "pass_hash TEXT NOT NULL,"
                  "salt TEXT NOT NULL)");

            db->exec("CREATE TABLE IF NOT EXISTS pairs ("
                    "a_username TEXT NOT NULL,"
                    "d_username TEXT NOT NULL,"
                    "app_id TEXT NOT NULL,"
                    "totp_secret TEXT NOT NULL,"
                    "PRIMARY KEY(d_username, app_id),"
                    "FOREIGN KEY(a_username) REFERENCES users(username) ON DELETE CASCADE)");
#else
            db->exec("CREATE TABLE IF NOT EXISTS users ("
                   "username TEXT PRIMARY KEY NOT NULL,"
                   "pass_hash TEXT NOT NULL,"
                   "salt TEXT NOT NULL)");
#endif
        } catch (std::exception& e) {
            std::cerr << "[DB Error] Init error: " << e.what() << "\n";
        }
    }

    bool createUser(const UserDTO &user) {
        if (!db) return false;
        try {
            SQLite::Statement query(*db, "INSERT INTO users (username, pass_hash, salt) VALUES (?, ?, ?)");
            query.bind(1, user.username);
            query.bind(2, user.pass_hash);
            query.bind(3, user.salt);
            query.exec();
            std::cout << "[DB Log] User created: " << user.username << "\n";
            return true;
        } catch (std::exception& e) {
            std::cerr << "[DB Error] Creating user failed: " << e.what() << "\n";
            return false;
        }
    }

    std::optional<UserDTO> getUser(const std::string &username) {
        if (!db) return std::nullopt;
        try {
            SQLite::Statement query(*db, "SELECT * FROM users WHERE username = ?");
            query.bind(1, username);

            query.executeStep();
            UserDTO user;
            user.username = query.getColumn(0).getString();
            user.pass_hash = query.getColumn(1).getString();
            user.salt = query.getColumn(2).getString();
            return user;
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Getting user failed: " << e.what() << "\n";
            return std::nullopt;
        }
    }

    bool pairUser(const std::string &a_username, const std::string &d_username,
                  const std::string &app_id, const std::string &secret) {
        if (!db) return true;
        try {
            SQLite::Statement query(*db, "INSERT INTO pairs (a_username, d_username, "
                                                "app_id, totp_secret) VALUES (?, ?, ?, ?)");
            query.bind(1, a_username);
            query.bind(2, d_username);
            query.bind(3, app_id);
            query.bind(4, secret);
            query.exec();
            std::cout << "[DB Log] Pairing created: " << a_username << " - " << d_username << "\n";
            return true;
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Pair creation failed: " << e.what() << "\n";
            return false;
        }
    }

    void updateSecret(const std::string &username, const std::string &secret) {
        if (!db) return;

        try {
            SQLite::Statement query(*db, "UPDATE users SET totp_secret = ? WHERE username = ?");
            query.bind(1, secret);
            query.bind(2, username);
            query.exec();
            std::cout << "[DB Log] Secret updated for user: " << username << "\n";
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Updating secret failed: " << e.what() << "\n";
        }
    }

    void removeUser(const std::string &username) {
        if (!db) return;

        try {
            SQLite::Statement query(*db, "DELETE FROM users WHERE username = ?");
            query.bind(1, username);
            query.exec();
            std::cout << "[DB Log] User removed: " << username << "\n";
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Removing user failed: " << e.what() << "\n";
        }
    }

    std::optional<std::string> getD_username(const std::string &a_username, const std::string &app_id) {
        if (!db) return std::nullopt;
        try {
            SQLite::Statement query(*db, "SELECT d_username FROM pairs WHERE a_username = ? AND app_id = ?");
            query.bind(1, a_username);
            query.bind(2, app_id);
            query.executeStep();
            std::cout << "[DB Log] Pair found: " << a_username << " - " << query.getColumn(0) << "\n";
            return query.getColumn(0);
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Pair lookup failed: " << e.what() << "\n";
            return std::nullopt;
        }
    }

    std::optional<std::string> getSecret(const std::string &d_username, const std::string &app_id) {
        if (!db) return std::nullopt;
        try {
            SQLite::Statement query(*db, "SELECT totp_secret FROM pairs WHERE d_username = ? AND app_id = ?");
            query.bind(1, d_username);
            query.bind(2, app_id);
            query.executeStep();
            std::cout << "[DB Log] Secret found: " << query.getColumn(0) << "\n";
            return query.getColumn(0);
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Secret lookup failed: " << e.what() << "\n";
            return std::nullopt;
        }
    }

    std::map<std::string, std::string> getSecretPairings(const std::string &username) {
        std::map<std::string, std::string> pairings;
        if (!db) return pairings;
        try {
            SQLite::Statement query(*db, "SELECT app_id, totp_secret FROM pairs WHERE a_username = ?");
            query.bind(1, username);
            while (query.executeStep()) {
                // app_id -> secret
                pairings[query.getColumn(0).getString()] = query.getColumn(1).getString();
            }
        } catch (std::exception &e) {
            std::cerr << "[DB Error] Secret pairings lookup failed: " << e.what() << "\n";
        }
        return pairings;
    }

    void show() {
        if (!db) return;

        std::cout << "[DB Log] Users:\n";
        SQLite::Statement query(*db, "SELECT * FROM users");
        while (query.executeStep()) {
            std::cout << "username = " << query.getColumn(0) << " | pass_hash = " << query.getColumn(1)
                << " | salt = " << query.getColumn(2) << "\n";
        }
#ifdef A_SERVER
        std::cout << "\n[DB Log] Pairings:\n";
        SQLite::Statement query2(*db, "SELECT * FROM pairs");
        while (query2.executeStep()) {
            std::cout << "a_username = " << query2.getColumn(0) << " | d_username = " << query2.getColumn(1)
                << " | app_id = " << query2.getColumn(2) << " | totp_secret = " << query2.getColumn(3) << "\n";
        }
#endif
    }
}