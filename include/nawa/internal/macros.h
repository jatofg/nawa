/**
 * \file macros.h
 * \brief Macros for frequently used patterns.
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

#ifndef NAWA_MACROS_H
#define NAWA_MACROS_H

#include <experimental/propagate_const>
#include <memory>

#define NAWA_PRIVATE_DATA() \
    struct Data;            \
    std::experimental::propagate_const<std::unique_ptr<Data>> data;

#define NAWA_DEFAULT_DESTRUCTOR_DEF(Class) virtual ~Class()
#define NAWA_DEFAULT_DESTRUCTOR_OVERRIDE_DEF(Class) ~Class() override
#define NAWA_DEFAULT_DESTRUCTOR_IMPL(Class) Class::~Class() = default;
#define NAWA_DEFAULT_DESTRUCTOR_IMPL_WITH_NS(Namespace, Class) Namespace::Class::~Class() = default;

#define NAWA_DEFAULT_CONSTRUCTOR_DEF(Class) Class()
#define NAWA_DEFAULT_CONSTRUCTOR_IMPL(Class) \
    Class::Class() { data = make_unique<Data>(); }
#define NAWA_DEFAULT_CONSTRUCTOR_IMPL_WITH_NS(Namespace, Class) \
    Namespace::Class::Class() { data = make_unique<Data>(); }

#define NAWA_COPY_CONSTRUCTOR_DEF(Class) Class(Class const& other)
#define NAWA_COPY_CONSTRUCTOR_IMPL(Class) \
    Class::Class(const Class& other) { data = make_unique<Data>(*other.data); }
#define NAWA_COPY_CONSTRUCTOR_IMPL_WITH_NS(Namespace, Class) \
    Namespace::Class::Class(const Namespace::Class& other) { data = make_unique<Data>(*other.data); }
#define NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL(Class, Parent) \
    Class::Class(const Class& other) : Parent(other) { data = make_unique<Data>(*other.data); }
#define NAWA_COPY_CONSTRUCTOR_DERIVED_IMPL_WITH_NS(Namespace, Class, Parent) \
    Namespace::Class::Class(const Namespace::Class& other) : Parent(other) { data = make_unique<Data>(*other.data); }

#define NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Class) Class& operator=(const Class& other)
#define NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Class) \
    Class& Class::operator=(const Class& other) { \
        if (this != &other) {                     \
            *data = *other.data;                  \
        }                                         \
        return *this;                             \
    }
#define NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(Namespace, Class)               \
    Namespace::Class& Namespace::Class::operator=(const Namespace::Class& other) { \
        if (this != &other) {                                                      \
            *data = *other.data;                                                   \
        }                                                                          \
        return *this;                                                              \
    }
#define NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL(Class, Parent) \
    Class& Class::operator=(const Class& other) {                 \
        if (this != &other) {                                     \
            Parent::operator=(other);                             \
            *data = *other.data;                                  \
        }                                                         \
        return *this;                                             \
    }
#define NAWA_COPY_ASSIGNMENT_OPERATOR_DERIVED_IMPL_WITH_NS(Namespace, Class, Parent) \
    Namespace::Class& Namespace::Class::operator=(const Namespace::Class& other) {   \
        if (this != &other) {                                                        \
            Parent::operator=(other);                                                \
            *data = *other.data;                                                     \
        }                                                                            \
        return *this;                                                                \
    }

#define NAWA_MOVE_CONSTRUCTOR_DEF(Class) Class(Class&& other) noexcept
#define NAWA_MOVE_CONSTRUCTOR_IMPL(Class) \
    Class::Class(Class&& other) noexcept : data(move(other.data)) {}
#define NAWA_MOVE_CONSTRUCTOR_IMPL_WITH_NS(Namespace, Class) \
    Namespace::Class::Class(Namespace::Class&& other) noexcept : data(move(other.data)) {}
#define NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL(Class, Parent) \
    Class::Class(Class&& other) noexcept : Parent(move(other)), data(move(other.data)) {}
#define NAWA_MOVE_CONSTRUCTOR_DERIVED_IMPL_WITH_NS(Namespace, Class, Parent) \
    Namespace::Class::Class(Namespace::Class&& other) noexcept : Parent(move(other)), data(move(other.data)) {}

#define NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Class) Class& operator=(Class&& other) noexcept
#define NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Class)     \
    Class& Class::operator=(Class&& other) noexcept { \
        if (this != &other) {                         \
            data = move(other.data);                  \
        }                                             \
        return *this;                                 \
    }
#define NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL_WITH_NS(Namespace, Class)                   \
    Namespace::Class& Namespace::Class::operator=(Namespace::Class&& other) noexcept { \
        if (this != &other) {                                                          \
            data = move(other.data);                                                   \
        }                                                                              \
        return *this;                                                                  \
    }
#define NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL(Class, Parent) \
    Class& Class::operator=(Class&& other) noexcept {             \
        if (this != &other) {                                     \
            data = move(other.data);                              \
            Parent::operator=(move(other));                       \
        }                                                         \
        return *this;                                             \
    }
#define NAWA_MOVE_ASSIGNMENT_OPERATOR_DERIVED_IMPL_WITH_NS(Namespace, Class, Parent)   \
    Namespace::Class& Namespace::Class::operator=(Namespace::Class&& other) noexcept { \
        if (this != &other) {                                                          \
            data = move(other.data);                                                   \
            Parent::operator=(move(other));                                            \
        }                                                                              \
        return *this;                                                                  \
    }

#define NAWA_PRIMITIVE_DATA_ACCESSORS_DEF(Class, Member, Type) \
    Type& Member() noexcept;                                   \
    [[nodiscard]] Type Member() const noexcept;                \
    Class& Member(Type value) noexcept
#define NAWA_COMPLEX_DATA_ACCESSORS_DEF(Class, Member, Type) \
    Type& Member() noexcept;                                 \
    [[nodiscard]] Type const& Member() const noexcept;       \
    Class& Member(Type value) noexcept
#define NAWA_PRIMITIVE_DATA_ACCESSORS_IMPL(Class, Member, Type)  \
    Type& Class::Member() noexcept { return data->Member; }      \
    Type Class::Member() const noexcept { return data->Member; } \
    Class& Class::Member(Type value) noexcept {                  \
        data->Member = value;                                    \
        return *this;                                            \
    }
#define NAWA_COMPLEX_DATA_ACCESSORS_IMPL(Class, Member, Type)           \
    Type& Class::Member() noexcept { return data->Member; }             \
    Type const& Class::Member() const noexcept { return data->Member; } \
    Class& Class::Member(Type value) noexcept {                         \
        data->Member = move(value);                                     \
        return *this;                                                   \
    }

#endif//NAWA_MACROS_H
