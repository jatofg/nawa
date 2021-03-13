/**
 * \file HandleRequestFunction.cpp
 * \brief Implementation of the HandleRequestFunction class.
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

#include <nawa/RequestHandler/HandleRequestFunction.h>

using namespace nawa;
using namespace std;

struct HandleRequestFunctionWrapper::Data {
    HandleRequestFunction handleRequestFunction;
    void *reference = nullptr;
    DestructionCallbackFunction destructionCallback;

    /**
     * Call the destruction callback on destruction of the data container (which should happen during destruction of
     * the HandleRequestFunctionWrapper object).
     */
    ~Data() {
        if (destructionCallback) {
            destructionCallback(reference);
        }
    }
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(HandleRequestFunctionWrapper)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(HandleRequestFunctionWrapper)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(HandleRequestFunctionWrapper, handleRequestFunction, HandleRequestFunction)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(HandleRequestFunctionWrapper, reference, void *)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(HandleRequestFunctionWrapper, destructionCallback, DestructionCallbackFunction)

nawa::HandleRequestFunctionWrapper::HandleRequestFunctionWrapper(HandleRequestFunction handleRequestFunction,
                                                                 void *reference,
                                                                 DestructionCallbackFunction destructionCallback)
        : HandleRequestFunctionWrapper() {
    data->handleRequestFunction = move(handleRequestFunction);
    data->reference = reference;
    data->destructionCallback = move(destructionCallback);
}

int nawa::HandleRequestFunctionWrapper::operator()(Connection &connection) const {
    return data->handleRequestFunction(connection);
}
