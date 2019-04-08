/**
 * \file Universal.h
 * \brief Contains the Universal type, a wrapper for objects on the heap.
 */

/*
 * Copyright (C) 2019 Jan Flaig.
 *
 * This file is part of QSF.
 *
 * QSF is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * QSF is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QSF.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef QSF_UNIVERSAL_H
#define QSF_UNIVERSAL_H

#include <typeinfo>
#include <typeindex>
#include <memory>
#include "UserException.h"

namespace Qsf {
    /**
     * Wrapper to store a variable or object of arbitrary type somewhere else on the heap. Some properties:
     *
     * (1) Type safety. While Universal can store an arbitrary type, accessing it requires explicitly stating it
     * as a template parameter. Type checking is done at runtime, an exception will be thrown when trying to
     * cast to the wrong type.
     * (2) Safe copying. Copying the object will also copy the attached object on the heap.
     * (3) Safe destruction. Memory will be freed when the destructor is called.
     */
    class Universal {
        std::type_index typeIndex; /**< Store type for comparison. */
        void* ptr; /**< Pointer to the piece of memory where the object is stored. */
        bool set; /**< Is an object currently stored in the Universal? */
        void (Universal::*deleter)(); /**< Pointer to destructor for the current object. */
        void (Universal::*copier)(const Universal&); /**< Pointer to copy function for the current object. */

        // save a deleter function to properly call stored object destructor in Universal destructor
        void deleteNothing() {}

        template<typename T>
        void deleteValue() {
            T* tPtr = (T*) ptr;
            delete tPtr;
        }

        // copy function
        void copyNothing(const Universal&) {}

        template<typename T>
        void copyValue(const Universal& from) {
            *this = from.get<T>();
        }
    public:
        /**
         * Create an empty, uninitialized Universal.
         */
        Universal() : typeIndex(typeid(void)), set(false), ptr(nullptr) {
            deleter = &Universal::deleteNothing;
            copier = &Universal::copyNothing;
        }

        // do not set set to true as operator= would call the deleter function then
        Universal(const Universal& value) : typeIndex(value.typeIndex), set(false), ptr(nullptr) {
            // members will be set by copier
            copier = value.copier;
            (this->*copier)(value);
        }

        /**
         * Construct a new Universal and assign an object of arbitrary type.
         * @tparam T Type of the object (can usually be derived automatically by the compiler).
         * @param value The object.
         */
        template<typename T>
        explicit Universal(T value) : typeIndex(typeid(value)), set(true) {
            T* tPtr = new T(std::move(value));
            ptr = (void*) tPtr;
            deleter = &Universal::deleteValue<T>;
            copier = &Universal::copyValue<T>;
        }

        Universal& operator=(const Universal& value) {
            // copier will deal with this
            copier = value.copier;
            (this->*copier)(value);
            return *this;
        }

        /**
         * Assign a new value (possibly of another type) to the Universal.
         * @tparam T Type of the new value (usually derived automatically by the compiler).
         * @param value The object.
         * @return Reference to the current Universal.
         */
        template<typename T>
        Universal& operator=(T value) {
            // delete current value first before assigning a new one
            if(set) {
                (this->*deleter)();
            }

            typeIndex = typeid(value);
            set = true;
            T* tPtr = new T(std::move(value));
            ptr = (void*) tPtr;
            deleter = &Universal::deleteValue<T>;
            copier = &Universal::copyValue<T>;

            return *this;
        }

        ~Universal() {
            (this->*deleter)();
        }

        /**
         * Get value of the Universal (copy of the stored object). If T differs from the type of the stored object,
         * a UserException with error code 2 will be thrown. On trying to access an uninitialized or unset
         * Universal, a UserException with error code 1 will be thrown.
         * @tparam T Type of the object. Must be provided explicitly.
         * @return Copy of the stored object.
         */
        template<typename T>
        T get() const {
            // throw an exception if no value set
            if(!set) {
                throw UserException("Qsf::Types::Universal::get<T>", 1, "Cast of void value requested");
            }
            // check for type equality and throw exception if not matching
            std::type_index Tindex(typeid(T));
            if(Tindex != typeIndex) {
                throw UserException("Qsf::Types::Universal::get<T>", 2, "Cast to wrong type requested");
            }
            return *(T*)ptr;
        }

        /**
         * Enable explicit casting. You can also get the object by casting to T instead of calling get<T>.
         * Casting will call get<T>, so the same exceptions will be thrown on type mismatch.
         * @tparam T Type of the object.
         * @return Copy of the stored object.
         */
        template<typename T>
        explicit operator T() const {
            return get<T>();
        }

        /**
         * Check if the Universal has been initialized and not been unset.
         * @return True if the Universal currently contains a valid object, false otherwise.
         */
        bool isSet() const {
            return set;
        }

        /**
         * Get the type of the currently stored object for information purposes.
         * @return type_index struct with information on the type.
         */
        std::type_index getType() const {
            return typeIndex;
        }

        /**
         * Remove stored object from the Universal and free the used heap memory.
         */
        void unset() {
            // call deleter only if a value has been set
            if(set) {
                (this->*deleter)();
                set = false;
            }
        }
    };
}

#endif //QSF_UNIVERSAL_H
