#include <iostream>
#include <memory>
//#include "AuthManager.hpp"
//#include <SQLiteCpp/SQLiteCpp.h>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    fs::path db_path = __FILE__;
    std::cout << db_path.parent_path() / "as.db" << "\n";
    //AuthManager am("as");
    // am.placeholder();
    // am.show();
    // am.testRemoveUser("admin");
    // am.testAddUser("admin", "admin", "a");
    //am.show();

    // if (am.loginUser("admin", "admin")) {
    //     std::cout << "Login successful!\n";
    // }
    // else std::cout << "Login failed!\n";
    //
    // if (am.loginUser("admin", "adin")) {
    //     std::cout << "Login successful!\n";
    // }
    // else std::cout << "Login failed!\n";

    return 0;
}