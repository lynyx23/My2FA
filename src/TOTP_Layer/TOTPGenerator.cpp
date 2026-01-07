#include "TOTPGenerator.hpp"
#include <endian.h>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <sstream>
#include <ctime>

namespace TOTPGenerator {
    std::string generateTOTP(const std::string &secret, const time_t customTime) {
        uint64_t constexpr INTERVAL = 30;
        uint32_t constexpr MODULO = 1000000;

        const uint64_t currentTimestamp = (customTime == 0) ? std::time(nullptr) : customTime;
        const uint64_t timeStep = currentTimestamp / INTERVAL;

        uint64_t const bigEndianTime = htobe64(timeStep);

        unsigned char hmac[EVP_MAX_MD_SIZE];
        unsigned int hmac_length = 0;

        HMAC(EVP_sha1(), secret.c_str(), static_cast<int>(secret.size()),
             reinterpret_cast<const unsigned char *>(&bigEndianTime), sizeof(bigEndianTime), hmac, &hmac_length);

        uint8_t const offset = hmac[hmac_length - 1] & 0xf;
        uint32_t const binary_code = (hmac[offset] & 0x7f) << 24
            | (hmac[offset + 1] & 0xff) << 16
            | (hmac[offset + 2] & 0xff) << 8
            | (hmac[offset + 3] & 0xff);
        uint32_t const code = binary_code % MODULO;

        std::stringstream ss;
        ss << std::setw(6) << std::setfill('0') << code;
        return ss.str();
    }

    uint32_t getRemainingSeconds() {
        return 30 - (std::time(nullptr) % 30);
    }

    bool verifyCode(const std::string &secret, const std::string &code) {
        // checks the code validity, taking into consideration a tolerance window
        if (secret.empty()) return false;

        const time_t currentTimestamp = std::time(nullptr);
        for (int i = -1; i <= 1; i++) {
            if (generateTOTP(secret, currentTimestamp + i * 30) == code) return true;
        }

        return false;
    }
} // namespace TOTPGenerator
