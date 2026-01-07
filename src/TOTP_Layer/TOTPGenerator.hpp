#ifndef MY2FA_TOTPGENERATOR_HPP
#define MY2FA_TOTPGENERATOR_HPP

#pragma once
#include <cstdint>
#include <string>

namespace TOTPGenerator {
    [[nodiscard]] std::string generateTOTP(const std::string& secret, time_t customTime = 0);
    [[nodiscard]] uint32_t getRemainingSeconds();
    [[nodiscard]] bool verifyCode(const std::string &secret, const std::string &code);

}

#endif // MY2FA_TOTPGENERATOR_HPP
