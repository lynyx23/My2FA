#include <iostream>
#include <memory>
#include "AuthManager.hpp"

int main() {
    AuthManager am("as");
    // am.placeholder();
    // am.show();
    // am.testRemoveUser("admin");
    am.testAddUser("test", "test", "CJNBQQ3E3L3OSFEF3LKSNN2DPRVQCE7I");
    am.show();

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