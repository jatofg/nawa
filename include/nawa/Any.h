/**
 * \file Any.h
 * \brief Contains the Any type, a wrapper for objects on the heap.
 */

/*
 * Copyright (C) 2019 Tobias Flaig.
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

#ifndef NAWA_ANY_H
#define NAWA_ANY_H

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <nawa/UserException.h>

namespace nawa {
    /**
     * Wrapper to store a variable or object of arbitrary type somewhere else on the heap. Some properties:
     *
     * (1) Type safety. While Any can store an arbitrary type, accessing it requires explicitly stating it
     * as a template parameter. Type checking is done at runtime, an exception will be thrown when trying to
     * cast to the wrong type.
     * (2) Safe copying. Copying the object will also copy the attached object on the heap.
     * (3) Safe destruction. Memory will be freed when the destructor is called.
     */
    class Any {
        std::type_index typeIndex; /**< Store type for comparison. */
        void* ptr; /**< Pointer to the piece of memory where the object is stored. */
        bool set; /**< Is an object currently stored in the Any? */
        void (Any::*deleter)(); /**< Pointer to destructor for the current object. */
        void (Any::*copier)(const Any&); /**< Pointer to copy function for the current object. */

        // save a deleter function to properly call stored object destructor in Any destructor
        void deleteNothing() {}

        template<typename T>
        void deleteValue() {
            T* tPtr = (T*) ptr;
            delete tPtr;
        }

        // copy function
        void copyNothing(const Any&) {}

        template<typename T>
        void copyValue(const Any& from) {
            *this = from.get<T>();
        }
    public:
        /**
         * Create an empty, uninitialized Any.
         */
        Any() : typeIndex(typeid(void)), set(false), ptr(nullptr) {
            deleter = &Any::deleteNothing;
            copier = &Any::copyNothing;
        }

        // do not set set to true as operator= would call the deleter function then
        Any(const Any& value) : typeIndex(value.typeIndex), set(false), ptr(nullptr) {
            // members will be set by copier
            copier = value.copier;
            (this->*copier)(value);
        }

        /**
         * Construct a new Any and assign an object of arbitrary type.
         * @tparam T Type of the object (can usually be derived automatically by the compiler).
         * @param value The object.
         */
        template<typename T>
        Any(T value) : typeIndex(typeid(value)), set(true) {
            T* tPtr = new T(std::move(value));
            ptr = (void*) tPtr;
            deleter = &Any::deleteValue<T>;
            copier = &Any::copyValue<T>;
        }

        /**
         * Special handling for string literals: Always save them as strings. It's always better, seriously.
         * @param value A string literal.
         */
        Any(const char* value) : typeIndex(typeid(std::string)), set(true) {
            std::string* tPtr = new std::string(value);
            ptr = (void*) tPtr;
            deleter = &Any::deleteValue<std::string>;
            copier = &Any::copyValue<std::string>;
        }

        Any& operator=(const Any& value) {
            // copier will deal with this
            copier = value.copier;
            (this->*copier)(value);
            return *this;
        }

        /**
         * Assign a new value (possibly of another type) to the Any.
         * @tparam T Type of the new value (usually derived automatically by the compiler).
         * @param value The object.
         * @return Reference to the current Any.
         */
        template<typename T>
        Any& operator=(T value) {
            // delete current value first before assigning a new one
            if(set) {
                (this->*deleter)();
            }

            typeIndex = typeid(value);
            set = true;
            T* tPtr = new T(std::move(value));
            ptr = (void*) tPtr;
            deleter = &Any::deleteValue<T>;
            copier = &Any::copyValue<T>;

            return *this;
        }

        ~Any() {
            (this->*deleter)();
        }

        /**
         * Get value of the Any (copy of the stored object). If T differs from the type of the stored object,
         * a UserException with error code 2 will be thrown. On trying to access an uninitialized or unset
         * Any, a UserException with error code 1 will be thrown.
         * @tparam T Type of the object. Must be provided explicitly.
         * @return Copy of the stored object.
         */
        template<typename T>
        T get() const {
            // throw an exception if no value set
            if(!set) {
                throw UserException("nawa::Types::Any::get<T>", 1, "Cast of void value requested");
            }
            // check for type equality and throw exception if not matching
            std::type_index Tindex(typeid(T));
            if(Tindex != typeIndex) {
                throw UserException("nawa::Types::Any::get<T>", 2, "Cast to wrong type requested");
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
         * Check if the Any has been initialized and not been unset.
         * @return True if the Any currently contains a valid object, false otherwise.
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
         * Remove stored object from the Any and free the used heap memory.
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

#endif //NAWA_ANY_H
