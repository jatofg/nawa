/**
 * \file HandleRequestFunction.h
 * \brief Function wrapper for handleRequest functions of nawa apps.
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

#ifndef NAWA_HANDLEREQUESTFUNCTION_H
#define NAWA_HANDLEREQUESTFUNCTION_H

#include <functional>
#include <nawa/internal/fwdecl.h>
#include <nawa/internal/macros.h>

namespace nawa {
    using HandleRequestFunction = std::function<int(nawa::Connection &)>;
    using DestructionCallbackFunction = std::function<void(void *)>;

    /**
     * A wrapper structure for a HandleRequestFunction which can store an additional reference pointer and a
     * callback, which will be called on destruction. This allows nawarun to free memory consumed by an app as
     * soon as it is not in use anymore (e.g., after being replaced in a hotswap operation).
     */
    class HandleRequestFunctionWrapper {
        NAWA_PRIVATE_DATA()

    public:
        NAWA_DEFAULT_DESTRUCTOR_DEF(HandleRequestFunctionWrapper);

        /**
         * Construct a wrapper for a HandleRequestFunction.
         * @param handleRequestFunction The request handling function.
         * @param reference An arbitrary reference pointer which can be used as a hint during destruction.
         * @param destructionCallback A function which will be called on destruction of the wrapper object, the
         * reference pointer is passed to this function as a reference.
         */
        explicit HandleRequestFunctionWrapper(HandleRequestFunction handleRequestFunction, void *reference = nullptr,
                                              DestructionCallbackFunction destructionCallback = DestructionCallbackFunction());

        /**
         * Convenience operator which just runs the request handling function.
         * @param connection The nawa::Connection object (parameter of the request handling function).
         * @return Return value of the request handling function.
         */
        int operator()(nawa::Connection &connection) const;

    };
}

#endif //NAWA_HANDLEREQUESTFUNCTION_H
