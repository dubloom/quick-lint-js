// Copyright (C) 2020  Matthew "strager" Glazar
// See end of file for extended copyright information.

#include <array>
#include <cstddef>
#include <cstring>
#include <gtest/gtest.h>
#include <quick-lint-js/container/string-view.h>
#include <quick-lint-js/port/warning.h>
#include <quick-lint-js/util/integer.h>
#include <string_view>
#include <system_error>

QLJS_WARNING_IGNORE_GCC("-Wsuggest-override")
QLJS_WARNING_IGNORE_GCC("-Wtype-limits")

using namespace std::literals::string_view_literals;

namespace quick_lint_js {
namespace {
using Test_Parse_Integer_Exact_Decimal_Types =
    ::testing::Types<unsigned short, int, std::size_t>;
template <class T>
class Test_Parse_Integer_Exact_Decimal : public ::testing::Test {};
TYPED_TEST_SUITE(Test_Parse_Integer_Exact_Decimal,
                 Test_Parse_Integer_Exact_Decimal_Types,
                 ::testing::internal::DefaultNameGenerator);

TYPED_TEST(Test_Parse_Integer_Exact_Decimal, common_non_negative_integers) {
  {
    TypeParam number;
    Parse_Integer_Exact_Error parse_error = parse_integer_exact("0"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
    EXPECT_EQ(number, 0);
  }

  {
    TypeParam number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("1234"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
    EXPECT_EQ(number, 1234);
  }
}

TEST(Test_Parse_Integer_Exact_Wchars_Decimal_Unsigned_Short, common_integers) {
  {
    unsigned short number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact(L"1234"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
    EXPECT_EQ(number, 1234);
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Int, common_negative_integers) {
  {
    int number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("-1234"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
    EXPECT_EQ(number, -1234);
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Int, minimum_integer) {
  static_assert(std::numeric_limits<int>::min() == -2147483648LL);
  int number;
  Parse_Integer_Exact_Error parse_error =
      parse_integer_exact("-2147483648"sv, number);
  EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
  EXPECT_EQ(number, -2147483648LL);
}

TEST(Test_Parse_Integer_Exact_Decimal_Unsigned_Short, maximum_integer) {
  static_assert(std::numeric_limits<unsigned short>::max() == 65535);
  unsigned short number;
  Parse_Integer_Exact_Error parse_error =
      parse_integer_exact("65535"sv, number);
  EXPECT_EQ(number, 65535);
  EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
}

TEST(Test_Parse_Integer_Exact_Decimal_Int, maximum_integer) {
  static_assert(std::numeric_limits<int>::max() == 2147483647);
  int number;
  Parse_Integer_Exact_Error parse_error =
      parse_integer_exact("2147483647"sv, number);
  EXPECT_EQ(number, 2147483647);
  EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
}

TEST(Test_Parse_Integer_Exact_Decimal_Size_T, maximum_integer) {
  static_assert(std::numeric_limits<std::size_t>::max() == 4294967295ULL ||
                std::numeric_limits<std::size_t>::max() ==
                    18446744073709551615ULL);

  {
    std::size_t number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("4294967295"sv, number);
    EXPECT_EQ(number, 4294967295ULL);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
  }

  if (std::numeric_limits<std::size_t>::max() >= 18446744073709551615ULL) {
    std::size_t number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("18446744073709551615"sv, number);
    EXPECT_EQ(number, 18446744073709551615ULL);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Unsigned_Short, exhausive_ok_SLOW) {
  static constexpr unsigned short max_ushort =
      std::numeric_limits<unsigned short>::max();
  static_assert(max_ushort < std::numeric_limits<unsigned>::max());
  for (unsigned i = 0; i <= max_ushort; ++i) {
    std::array<char, integer_string_length<unsigned short>> buffer;
    char* end = write_integer(i, buffer.data());
    std::string_view string = make_string_view(buffer.data(), end);
    SCOPED_TRACE(string);

    unsigned short parsed_number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact(string, parsed_number);
    EXPECT_EQ(parsed_number, i);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Unsigned_Short, over_maximum_integer) {
  static_assert(std::numeric_limits<unsigned short>::max() < 65536);

  {
    unsigned short number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("65536"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    unsigned short number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("9999999999999999999"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  // These out-of-range examples might trick a naive overflow check. For
  // example: (7281*10 + 7)%(1<<16) > 7281
  for (std::string_view input : {"72817"sv, "72820"sv}) {
    SCOPED_TRACE(input);
    unsigned short number = 42;
    Parse_Integer_Exact_Error parse_error = parse_integer_exact(input, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  // These out-of-range examples might trick a naive overflow check. For
  // example: (43822*10 + 3)%(1<<16) > 43822
  for (std::string_view input : {"100000"sv, "438223"sv, "655369"sv}) {
    SCOPED_TRACE(input);
    unsigned short number = 42;
    Parse_Integer_Exact_Error parse_error = parse_integer_exact(input, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Int, over_maximum_integer) {
  static_assert(std::numeric_limits<int>::max() < 2147483648LL);

  {
    int number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("2147483648"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    int number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("9999999999999999999"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Size_T, over_maximum_integer) {
  static_assert(std::numeric_limits<std::size_t>::max() == 4294967295ULL ||
                std::numeric_limits<std::size_t>::max() ==
                    18446744073709551615ULL);

  if (std::numeric_limits<std::size_t>::max() <= 4294967295ULL) {
    std::size_t number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("4294967296"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    std::size_t number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("18446744073709551616"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    int number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("9999999999999999999999"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::out_of_range);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TEST(Test_Parse_Integer_Exact_Decimal_Size_T,
     negative_integers_are_disallowed) {
  {
    std::size_t number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("-9001"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TYPED_TEST(Test_Parse_Integer_Exact_Decimal,
           extra_characters_after_are_not_parsed) {
  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("1234abcd"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("123   "sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TYPED_TEST(Test_Parse_Integer_Exact_Decimal, extra_characters_before) {
  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("  123"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("--123"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("+123"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }
}

TYPED_TEST(Test_Parse_Integer_Exact_Decimal, radix_prefix_is_not_special) {
  {
    TypeParam number = 42;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("0x123a"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
    EXPECT_EQ(number, 42) << "number should be unmodified";
  }

  {
    TypeParam number;
    Parse_Integer_Exact_Error parse_error =
        parse_integer_exact("0777"sv, number);
    EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::ok);
    EXPECT_EQ(number, 777);
  }
}

TYPED_TEST(Test_Parse_Integer_Exact_Decimal,
           empty_input_string_is_unrecognized) {
  TypeParam number = 42;
  Parse_Integer_Exact_Error parse_error = parse_integer_exact(""sv, number);
  EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
  EXPECT_EQ(number, 42) << "number should be unmodified";
}

TYPED_TEST(Test_Parse_Integer_Exact_Decimal,
           minus_sign_without_digits_is_unrecognized) {
  TypeParam number = 42;
  Parse_Integer_Exact_Error parse_error = parse_integer_exact("- 1"sv, number);
  EXPECT_EQ(parse_error, Parse_Integer_Exact_Error::invalid);
  EXPECT_EQ(number, 42) << "number should be unmodified";
}
}
}

// quick-lint-js finds bugs in JavaScript programs.
// Copyright (C) 2020  Matthew "strager" Glazar
//
// This file is part of quick-lint-js.
//
// quick-lint-js is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// quick-lint-js is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with quick-lint-js.  If not, see <https://www.gnu.org/licenses/>.
