/***************************************************************************
 * Copyright (c) 2024 by gempa GmbH                                        *
 *
 * All Rights Reserved.                                                    *
 *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *
 * Author: Thomas Bornstein                                                *
 * Email: thomas.bornstein@gempa.de                                        *
 ***************************************************************************/


#ifndef CHARCONV_FLOAT_CHARCONV_H
#define	CHARCONV_FLOAT_CHARCONV_H

#include <charconv>

#if defined __GLIBCXX__ && _GLIBCXX_RELEASE < 11 && __cplusplus >= 201703L

#if __cpp_lib_to_chars < 201611L && _GLIBCXX_HAVE_USELOCALE
#	undef __cpp_lib_to_chars
#	define __cpp_lib_to_chars 201611L
#endif



namespace std::backports
{
  // floating-point format for primitive numerical conversion
  enum class chars_format {
    scientific = 1,
    fixed = 2,
    hex = 4,
    general = fixed | scientific
  };

constexpr chars_format
  operator|(chars_format __lhs, chars_format __rhs) noexcept
  { return (chars_format)((unsigned)__lhs | (unsigned)__rhs); }

  constexpr chars_format
  operator&(chars_format __lhs, chars_format __rhs) noexcept
  { return (chars_format)((unsigned)__lhs & (unsigned)__rhs); }

  constexpr chars_format
  operator^(chars_format __lhs, chars_format __rhs) noexcept
  { return (chars_format)((unsigned)__lhs ^ (unsigned)__rhs); }

  constexpr chars_format
  operator~(chars_format __fmt) noexcept
  { return (chars_format)~(unsigned)__fmt; }

  constexpr chars_format&
  operator|=(chars_format& __lhs, chars_format __rhs) noexcept
  { return __lhs = __lhs | __rhs; }

  constexpr chars_format&
  operator&=(chars_format& __lhs, chars_format __rhs) noexcept
  { return __lhs = __lhs & __rhs; }

  constexpr chars_format&
  operator^=(chars_format& __lhs, chars_format __rhs) noexcept
  { return __lhs = __lhs ^ __rhs; }



    /// Result type of std::from_chars
  struct from_chars_result
  {
    const char* ptr;
    errc ec;

#if __cplusplus > 201703L && __cpp_impl_three_way_comparison >= 201907L
    friend bool
    operator==(const from_chars_result&, const from_chars_result&) = default;
#endif
  };

auto from_chars(const char* first, const char* last, float& value, chars_format fmt = chars_format::general) noexcept -> from_chars_result;
auto from_chars(const char* first, const char* last, double& value, chars_format fmt = chars_format::general) noexcept -> from_chars_result;
auto from_chars(const char* first, const char* last, long double& value, chars_format fmt = chars_format::general) noexcept -> from_chars_result;
} // namespace backports

#endif // defined __GLIBCXX__ && _GLIBCXX_RELEASE < 11 && __cplusplus >= 201703L

#endif // CHARCONV_FLOAT_CHARCONV_H
