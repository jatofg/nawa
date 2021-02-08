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

#define NAWA_PRIVATE_IMPL_DEF() struct Impl; std::experimental::propagate_const<std::unique_ptr<Impl>> impl;

#define NAWA_DEFAULT_DESTRUCTOR_DEF(Class) virtual ~Class();
#define NAWA_DEFAULT_DESTRUCTOR_IMPL(Class) Class::~Class() = default;

#define NAWA_COPY_CONSTRUCTOR_DEF(Class) Class(Class const &other);
#define NAWA_COPY_CONSTRUCTOR_IMPL(Class) Class::Class(const Class &other) { impl = make_unique<Impl>(*other.impl); }

#define NAWA_COPY_ASSIGNMENT_OPERATOR_DEF(Class) Class &operator=(const Class &other);
#define NAWA_COPY_ASSIGNMENT_OPERATOR_IMPL(Class) Class &Class::operator=(const Class &other) { if (this != &other) { impl = make_unique<Impl>(*other.impl); } return *this; }

#define NAWA_MOVE_CONSTRUCTOR_DEF(Class) Class(Class &&other) noexcept;
#define NAWA_MOVE_CONSTRUCTOR_IMPL(Class) Class::Class(Class &&other) noexcept: impl(move(other.impl)) {}

#define NAWA_MOVE_ASSIGNMENT_OPERATOR_DEF(Class) Class &operator=(Class &&other) noexcept;
#define NAWA_MOVE_ASSIGNMENT_OPERATOR_IMPL(Class) Class &Class::operator=(Class &&other) noexcept { if (this != &other) { impl = move(other.impl); } return *this; }

#define NAWA_DEFAULT_CONSTRUCTOR_DEF(Class) Class();
#define NAWA_DEFAULT_CONSTRUCTOR_IMPL(Class) Class::Class() { impl = make_unique<Impl>(); }

#endif //NAWA_MACROS_H
