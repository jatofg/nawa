/*
 * Copyright (C) 2019-2022 Tobias Flaig.
 *
 * This file is part of nawa.
 *
 * nawa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * nawa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nawa.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * \file Config.h
 * \brief Reader for config files and accessor to config values.
 */

#ifndef NAWA_CONFIG_H
#define NAWA_CONFIG_H

#include <nawa/internal/macros.h>
#include <string>
#include <vector>

namespace nawa {
    /**
     * Reader for config files and accessor to config values.
     */
    class Config {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(Config);

        NAWA_DEFAULT_CONSTRUCTOR_DEF(Config);

        NAWA_COPY_CONSTRUCTOR_DEF(Config);

        NAWA_MOVE_CONSTRUCTOR_DEF(Config);

        NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Config);

        NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Config);

        /**
         * Construct Config container and directly parse an ini file. Throws an Exception on failure.
         * @param iniFile ini file to parse, values will be added to the Config container.
         */
        explicit Config(std::string const& iniFile);

        /**
         * Construct Config container based on values from an initializer list.
         * @param init Initializer list containing key-value pairs, wherein the keys are section-key pairs.
         */
        Config(std::initializer_list<std::pair<std::pair<std::string, std::string>, std::string>> init);

        /**
         * Read an ini file and add the values to the Config container. If a key is already set, the element will be
         * ignored, not overwritten. Throws a SysException on failure.
         * @param iniFile ini file to parse and import values from.
         */
        void read(std::string const& iniFile);

        /**
         * Insert values from an initializer list. If a key is already set, the element will be ignored, not
         * overwritten.
         * @param init Initializer list containing key-value pairs, wherein the keys are section-key pairs.
         */
        void insert(std::initializer_list<std::pair<std::pair<std::string, std::string>, std::string>> init);

        /**
         * Sets the elements form the vector to the given values (or inserts them, if they did not exist yet).
         * @param overrides Vector containing key-value pairs, wherein the keys are section-key pairs.
         */
        void override(std::vector<std::pair<std::pair<std::string, std::string>, std::string>> const& overrides);

        /**
         * Check whether a key exists in this Config container.
         * @param key Key (pair of section and name of the value) to check for.
         * @return True if the key exists, false if not.
         */
        [[nodiscard]] bool isSet(std::pair<std::string, std::string> const& key) const;

        /**
         * Get the value belonging to the specified key from the Config container.
         * @param key Key (pair of section and name of the value).
         * @return The value belonging to the key if it exists, an empty string otherwise (for distinguishing between
         * an empty value and a non-existing one, use the isSet() function).
         */
        std::string operator[](std::pair<std::string, std::string> const& key) const;

        /**
         * Set a key to a new value or insert a new key with the given value.
         * @param key Pair of section and key string identifying the Config value that is to be set.
         * @param value The value to set.
         */
        void set(std::pair<std::string, std::string> key, std::string value);

        /**
         * Set a key to a new value or insert a new key with the given value.
         * @param section The config section in which the key is located.
         * @param key The key.
         * @param value The value to set.
         */
        void set(std::string section, std::string key, std::string value);
    };
}// namespace nawa

#endif//NAWA_CONFIG_H
