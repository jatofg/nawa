//
// Created by tobias on 26/11/18.
//

#ifndef QSF_CONFIG_H
#define QSF_CONFIG_H

#include <unordered_map>
#include <utility>
#include <boost/functional/hash.hpp>

namespace Qsf {
    class Config {
        std::unordered_map<std::pair<std::string, std::string>, std::string, boost::hash<std::pair<std::string, std::string>>> values;
    public:
        /**
         * Construct empty Config container.
         */
        Config() = default;
        /**
         * Construct Config container and directly parse an ini file. Throws a SysException on failure.
         * @param iniFile ini file to parse, values will be added to the Config container.
         */
        explicit Config(std::string iniFile);
        virtual ~Config() = default;
        /**
         * Read an ini file and add the values to the Config container. Throws a SysException on failure.
         * @param iniFile ini file to parse and import values from.
         */
        void read(std::string iniFile);
        /**
         * Check whether a key exists in this Config container.
         * @param key Key (pair of section and name of the value) to check for.
         * @return True if the key exists, false if not.
         */
        bool isSet(std::pair<std::string, std::string> key) const;
        /**
         * Get the value belonging to the specified key from the Config container.
         * @param key Key (pair of section and name of the value).
         * @return The value belonging to the key if it exists, an empty string otherwise (for distinguishing between
         * an empty value and a non-existing one, use the isSet() function).
         */
        std::string operator[](std::pair<std::string, std::string> key) const;
    };
}

#endif //QSF_CONFIG_H
