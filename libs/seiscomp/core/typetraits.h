/***************************************************************************
 * Copyright (C) gempa GmbH                                                *
 * All rights reserved.                                                    *
 * Contact: gempa GmbH (seiscomp-dev@gempa.de)                             *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 *                                                                         *
 * Other Usage                                                             *
 * Alternatively, this file may be used in accordance with the terms and   *
 * conditions contained in a signed written agreement between you and      *
 * gempa GmbH.                                                             *
 ***************************************************************************/


#include <type_traits>
#include <string>
#include <string_view>


namespace Seiscomp::Core::TypeTraits {


template <typename T>
struct IsString : std::false_type {};

template <>
struct IsString<std::string> : std::true_type {};

template <>
struct IsString<std::string_view> : std::true_type {};

template <>
struct IsString<char*> : std::true_type {};

template <>
struct IsString<const char*> : std::true_type {};

using IsStdString = IsString<std::decay_t<std::string>>;

template <typename T>
using IsStringLike = IsString<std::decay_t<T>>;

/**
 * @brief Trait for parameter types that may be emitted into an SQL statement
 *        *without* quoting or escaping, i.e. whose textual representation can
 *        never contain SQL-significant characters.
 *
 * This is restricted to arithmetic types but explicitly excludes the character
 * types: a char holds an arbitrary byte (e.g. a single quote), so emitting it
 * unquoted would be an injection vector. Character data must go through the
 * string-like path (escape + quote) instead.
 */
template <typename T>
struct IsSQLNumber : std::bool_constant<
	std::is_arithmetic_v<std::decay_t<T>> &&
	!std::is_same_v<std::decay_t<T>, char> &&
	!std::is_same_v<std::decay_t<T>, signed char> &&
	!std::is_same_v<std::decay_t<T>, unsigned char> &&
	!std::is_same_v<std::decay_t<T>, wchar_t> &&
	!std::is_same_v<std::decay_t<T>, char16_t> &&
	!std::is_same_v<std::decay_t<T>, char32_t>
> {};


}
