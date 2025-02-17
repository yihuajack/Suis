/*
 * Copyright (C) 2024  Yihua Liu <yihuajack@live.cn>
 * Copyright (C) 2016-2023  Mike Tzou (Chocobo1)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#pragma once

#include <complex>
#include <type_traits>

template <typename T>
constexpr std::add_const_t<T> &asConst(T &t) noexcept { return t; }

// Forward rvalue as const
template <typename T>
constexpr std::add_const_t<T> asConst(T &&t) noexcept { return std::forward<T>(t); }

// Prevent const rvalue arguments
template <typename T>
void asConst(const T &&) = delete;

template<typename T>
concept IsComplex = std::is_same_v<T, std::complex<typename T::value_type>>;

template<typename T>
concept FloatingList = requires(T t) {
    // { t.size() } -> std::same_as<std::size_t>;
    // { t.empty() } -> std::same_as<bool>;
    { t[0] } -> std::convertible_to<typename std::remove_reference_t<T>::value_type>;
    // { t.push_back(typename T::value_type{}) };
} && std::floating_point<typename std::remove_reference_t<T>::value_type>;

template <typename T>
concept Vector = requires(T t) {
    { t.begin() } -> std::forward_iterator;
    { t.end() } -> std::forward_iterator;
    { t.empty() } -> std::same_as<bool>;
    { t[std::declval<std::size_t>()] } -> std::same_as<typename T::reference>;
    { t.front() } -> std::same_as<typename T::reference>;
    { t.back() } -> std::same_as<typename T::reference>;
    { t.data() } -> std::same_as<typename T::pointer>;
};
