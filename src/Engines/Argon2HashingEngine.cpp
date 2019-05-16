#include <qsf/Engines/Argon2HashingEngine.h>
#include <cstring>
#include <cstdlib>
#include <argon2.h>
#include <qsf/UserException.h>
#include <random>

Qsf::Engines::Argon2HashingEngine::Argon2HashingEngine(Qsf::Engines::Argon2HashingEngine::Algorithm algorithm,
                                                       uint32_t timeCost, uint32_t memoryCost, uint32_t parallelism,
                                                       std::string _salt, size_t hashLen)
        : algorithm(algorithm), timeCost(timeCost), memoryCost(memoryCost), parallelism(parallelism), hashLen(hashLen) {
    salt = std::move(_salt);
}

std::string Qsf::Engines::Argon2HashingEngine::generateHash(std::string input) const {

    // check validity of parameters
    if(!salt.empty() && salt.length() < ARGON2_MIN_SALT_LENGTH) {
        throw UserException("Qsf::Engines::Argon2HashingEngine::generateHash", 10,
                "Provided user-defined salt is not long enough");
    }

    std::string actualSalt = salt;
    if(salt.empty()) {
        // generate random salt (16 bytes)
        std::random_device rd;
        std::stringstream sstr;
        if(rd.entropy() == 32) {
            std::uniform_int_distribution<uint32_t> distribution(0, 0xffffffff);
            for(int i = 0; i < 4; ++i) {
                uint32_t val = distribution(rd);
                sstr << (char)(val & 0xff);
                sstr << (char)((val >> 8) & 0xff);
                sstr << (char)((val >> 16) & 0xff);
                sstr << (char)((val >> 24) & 0xff);
            }
        }
        else {
            std::uniform_int_distribution<uint16_t> distribution(0, 0xffff);
            for(int i = 0; i < 8; ++i) {
                uint16_t val = distribution(rd);
                sstr << (char)(val & 0xff);
                sstr << (char)((val >> 8) & 0xff);
            }
        }
        actualSalt = sstr.str();
    }

    int errorCode = 0;
    size_t encodedHashCeil = 50 + (actualSalt.length()*4)/3 + (hashLen*4)/3;
    char c_hash[encodedHashCeil];
    memset(c_hash, '\0', encodedHashCeil);

    switch(algorithm) {
        case ARGON2I:
            errorCode = argon2i_hash_encoded(timeCost, memoryCost, parallelism, (void*)input.c_str(), input.length(),
                                             (void*)actualSalt.c_str(), actualSalt.length(), hashLen, c_hash, encodedHashCeil);
            break;
        case ARGON2D:
            errorCode = argon2d_hash_encoded(timeCost, memoryCost, parallelism, (void*)input.c_str(), input.length(),
                                             (void*)actualSalt.c_str(), actualSalt.length(), hashLen, c_hash, encodedHashCeil);
            break;
        case ARGON2ID:
            errorCode = argon2id_hash_encoded(timeCost, memoryCost, parallelism, (void*)input.c_str(), input.length(),
                                             (void*)actualSalt.c_str(), actualSalt.length(), hashLen, c_hash, encodedHashCeil);
            break;
    }

    // error handling
    if(errorCode != ARGON2_OK) {
        throw UserException("Qsf::Engines::Argon2HashingEngine::generateHash", 10,
                std::string("Argon2 error: ") + argon2_error_message(errorCode));
    }

    return std::string(c_hash);
}

bool Qsf::Engines::Argon2HashingEngine::verifyHash(std::string input, std::string hash) const {

    // check existence and validity of the salt
    if(salt.empty() || salt.length() < ARGON2_MIN_SALT_LENGTH) {
        return false;
    }

    std::string inputHash;
    try {
        inputHash = generateHash(input);
    }
    catch(const UserException&) {
        return false;
    }

    if(inputHash.length() != hash.length())
        return false;

    auto u1 = (const unsigned char*) inputHash.c_str();
    auto u2 = (const unsigned char*) hash.c_str();

    int ret = 0;
    for (int i = 0; i < hash.length(); ++i)
        ret |= (u1[i] ^ u2[i]);

    return ret == 0;
}
