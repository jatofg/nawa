/**
 * \file nawarun.h
 * \brief Definitions for the nawarun implementation.
 */

/*
 * Copyright (C) 2019-2021 Tobias Flaig.
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

#ifndef NAWA_NAWARUN_H
#define NAWA_NAWARUN_H

#include <memory>
#include <nawa/internal/fwdecl.h>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace nawarun {
    using PrivilegeDowngradeData = std::tuple<uid_t, gid_t, std::vector<gid_t>>;

    struct Parameters {
        using ConfigOptionId = std::pair<std::string, std::string>;
        using ConfigOverride = std::pair<ConfigOptionId, std::string>;
        std::optional<std::string> configFile;
        std::vector<ConfigOverride> configOverrides;
    };

    // Types of functions that need to be accessed from NAWA applications
    using init_t = int(nawa::AppInit&);             /**< Type for the init() function of NAWA apps. */
    using handleRequest_t = int(nawa::Connection&); /**< Type for the handleRequest(Connection) function of NAWA apps. */

    /**
     * Get the number of threads to use from config.
     * @param config Config
     * @return Number of threads
     */
    unsigned int getConcurrency(nawa::Config const& config);

    /**
     * Load the init() and handleRequest() functions of an app. Note: the init function also becomes unavailable when
     * the HandleRequestFunctionWrapper is destructed. Throws a nawa::Exception on failure.
     * @param config Config
     * @return Pair of init function and smart pointer to wrapper containing the handleRequest function.
     */
    std::pair<init_t*, std::shared_ptr<nawa::HandleRequestFunctionWrapper>> loadAppFunctions(nawa::Config const& config);

    /**
     * Reload on signal.
     * @param signum Signal
     */
    void reload(int signum);

    /**
     * Prepare privilege downgrade considering the corresponding options in the config and check for errors. Will log
     * warnings and throw a nawa::Exception in case of failure.
     * @param config Config
     * @return A tuple with the UID and GID and a list of supplementary groups if privilege downgrade should happen
     * (i.e., nawarun is running as root).
     */
    std::optional<PrivilegeDowngradeData> preparePrivilegeDowngrade(nawa::Config const& config);

    /**
     * Replace nawarun's logger with the given one for testing purposes.
     * @param log Logger
     */
    void replaceLogger(nawa::Log const& log);

    /**
     * Parse command line arguments. If --help or -h is encountered as the first argument, printHelpAndExit() is invoked.
     * @param argc Number of arguments
     * @param argv Arguments
     * @return A structure containing the path to the config file (optional, as it can be skipped with --no-config-file), and
     * a list of config option overrides (identifier-value pairs, wherein the identifier is a pair of category and key).
     */
    Parameters parseCommandLine(int argc, char** argv);

    /**
     * Run nawarun.
     * @param parameters Parameters
     * @return Return code (0 on success, 1 on failure).
     */
    int run(Parameters const& parameters);
}// namespace nawarun

#endif//NAWA_NAWARUN_H
