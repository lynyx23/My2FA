#ifndef MY2FA_ENTITYTYPE_HPP
#define MY2FA_ENTITYTYPE_HPP

#pragma once
#include <cstdint>
#include <iostream>

// cleaner than using unsigned char
enum class EntityType : uint8_t {
    AUTH_SERVER = 0,
    AUTH_CLIENT = 1,
    DUMMY_SERVER = 2,
    DUMMY_CLIENT = 3,
    NOT_ASSIGNED = 4
};

inline std::ostream& operator<<(std::ostream& os, const EntityType& type) {
    switch (type) {
        case EntityType::AUTH_SERVER: return os << "AUTH_SERVER";
        case EntityType::AUTH_CLIENT: return os << "AUTH_CLIENT";
        case EntityType::DUMMY_SERVER: return os << "DUMMY_SERVER";
        case EntityType::DUMMY_CLIENT: return os << "DUMMY_CLIENT";
        default: return os << "NOT_ASSIGNED";
    }
}

#endif //MY2FA_ENTITYTYPE_HPP