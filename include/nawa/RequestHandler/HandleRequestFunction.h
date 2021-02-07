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

namespace nawa {
    // forward declarations
    class Connection;

    using HandleRequestFunction = std::function<int(nawa::Connection &)>;
    using DestructionCallbackFunction = std::function<void(void *)>;

    /**
     * A wrapper structure for a HandleRequestFunction which can store an additional reference pointer and a
     * callback, which will be called on destruction. This allows nawarun to free memory consumed by an app as
     * soon as it is not in use anymore (e.g., after being replaced in a hotswap operation).
     */
    struct HandleRequestFunctionWrapper {
        HandleRequestFunction handleRequestFunction; /**< The request handling function. */
        void *reference; /**< An arbitrary reference pointer which can be used as a hint during destruction. */
        /**
         * A function which will be called on destruction of the wrapper object, the reference pointer is passed to
         * this function as a reference.
         */
        DestructionCallbackFunction destructionCallback;

        /**
         * Construct an empty wrapper object.
         */
        HandleRequestFunctionWrapper() : reference(nullptr) {}

        /**
         * Construct a wrapper for a HandleRequestFunction.
         * @param handleRequestFunction The request handling function.
         * @param reference An arbitrary reference pointer which can be used as a hint during destruction.
         * @param destructionCallback The destruction callback function.
         */
        explicit HandleRequestFunctionWrapper(HandleRequestFunction handleRequestFunction, void *reference = nullptr,
                                              DestructionCallbackFunction destructionCallback = DestructionCallbackFunction())
                : handleRequestFunction(move(handleRequestFunction)), reference(reference),
                  destructionCallback(move(destructionCallback)) {}

        virtual ~HandleRequestFunctionWrapper() {
            if (destructionCallback) {
                destructionCallback(reference);
            }
        }

        HandleRequestFunctionWrapper(const HandleRequestFunctionWrapper &) = delete;

        HandleRequestFunctionWrapper &operator=(const HandleRequestFunctionWrapper &) = delete;

        /**
         * Convenience operator which just runs the request handling function.
         * @param connection The nawa::Connection object (parameter of the request handling function).
         * @return Return value of the request handling function.
         */
        int operator()(nawa::Connection &connection) const { return handleRequestFunction(connection); }

    };
}

#endif //NAWA_HANDLEREQUESTFUNCTION_H
