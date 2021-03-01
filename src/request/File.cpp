/**
 * \file File.cpp
 * \brief Implementation of the File class.
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

#include <nawa/request/File.h>

using namespace nawa;
using namespace std;

struct File::Data {
    string filename;
    string contentType;
    size_t size = 0;
    shared_ptr<char[]> dataPtr;
};

NAWA_DEFAULT_DESTRUCTOR_IMPL(File)

NAWA_DEFAULT_CONSTRUCTOR_IMPL(File)

NAWA_COPY_CONSTRUCTOR_IMPL(File)

NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(File)

NAWA_MOVE_CONSTRUCTOR_IMPL(File)

NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(File)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(File, filename, string)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(File, contentType, string)

NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(File, size, size_t)

NAWA_COMPLEX_DATA_ACCESSORS_IMPL(File, dataPtr, shared_ptr<char[]>)

string File::copyFile() const {
    return string(data->dataPtr.get(), data->size);
}

bool File::writeFile(const string &path) const {
    ofstream outfile;
    ios_base::iostate exceptionMask = outfile.exceptions() | ios::failbit;
    outfile.exceptions(exceptionMask);
    try {
        outfile.open(path, ofstream::out | ofstream::binary);
        outfile.write(data->dataPtr.get(), data->size);
        outfile.close();
    }
    catch (ios_base::failure &e) {
        return false;
    }
    return true;
}
