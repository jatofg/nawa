
#include <qsf/Engines/DefaultHashTypeTable.h>
#include <qsf/Engines/BcryptHashingEngine.h>

std::shared_ptr<Qsf::Engines::HashingEngine> Qsf::Engines::DefaultHashTypeTable::getEngine(std::string hash) const {
    auto hid = hash.substr(0, 4);
    if(hid == "$2a$" || hid == "$2b$" || hid == "$2x$" || hid == "$2y$") {
        return std::shared_ptr<Qsf::Engines::HashingEngine>(new Qsf::Engines::BcryptHashingEngine());
    }
    return std::shared_ptr<Qsf::Engines::HashingEngine>();
}
