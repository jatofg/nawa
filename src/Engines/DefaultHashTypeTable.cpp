
#include <qsf/Engines/DefaultHashTypeTable.h>
#include <qsf/Engines/BcryptHashingEngine.h>
#include <qsf/Engines/Argon2HashingEngine.h>

std::shared_ptr<Qsf::Engines::HashingEngine> Qsf::Engines::DefaultHashTypeTable::getEngine(std::string hash) const {
    auto hid = hash.substr(0, 4);
    if(hid == "$2a$" || hid == "$2b$" || hid == "$2x$" || hid == "$2y$") {
        return std::shared_ptr<Qsf::Engines::HashingEngine>(new Qsf::Engines::BcryptHashingEngine());
    }
    else if(hash.substr(0, 10) == "$argon2id$" || hash.substr(0, 9) == "$argon2i$" || hash.substr(0, 9) == "$argon2d$") {
        return std::shared_ptr<Qsf::Engines::HashingEngine>(new Qsf::Engines::Argon2HashingEngine());
    }
    return std::shared_ptr<Qsf::Engines::HashingEngine>();
}
