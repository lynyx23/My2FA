#include <iostream>
#include <memory>
#include "AuthManager.hpp"

int main() {
    std::unique_ptr<AuthManager> am = std::make_unique<AuthManager>("ds");
    // am->testRemoveUser("admin");
    am->testAddUser("admin", "admin");
    am->show();

    if (am->loginUser("admin", "admin")) {
        std::cout << "Login successful!\n";
    }
    else std::cout << "Login failed!\n";

    if (am->loginUser("admin", "adin")) {
        std::cout << "Login successful!\n";
    }
    else std::cout << "Login failed!\n";

    return 0;
}