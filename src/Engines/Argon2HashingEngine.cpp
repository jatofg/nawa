#include <qsf/Engines/Argon2HashingEngine.h>
#include <argon2.h>

Qsf::Engines::Argon2HashingEngine::Argon2HashingEngine(Qsf::Engines::Argon2HashingEngine::Algorithm algorithm,
                                                       uint32_t timeCost, uint32_t memoryCost, uint32_t parallelism,
                                                       std::string _salt, std::string _secret)
        : algorithm(algorithm), timeCost(timeCost), memoryCost(memoryCost), parallelism(parallelism) {
    salt = std::move(_salt);
    secret = std::move(_secret);
}

std::string Qsf::Engines::Argon2HashingEngine::generateHash(std::string input) const {
    return std::__cxx11::string();
}

bool Qsf::Engines::Argon2HashingEngine::verifyHash(std::string input, std::string hash) const {
    return false;
}
