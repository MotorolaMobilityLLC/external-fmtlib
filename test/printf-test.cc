/*
 printf tests.

 Copyright (c) 2012-2014, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <climits>
#include <cstring>

#include "format.h"
#include "gtest-extra.h"
#include "util.h"

using fmt::format;
using fmt::FormatError;

const unsigned BIG_NUM = INT_MAX + 1u;

// Makes format string argument positional.
std::string make_positional(fmt::StringRef format) {
  std::string s(format);
  s.replace(s.find('%'), 1, "%1$");
  return s;
}

#define EXPECT_PRINTF(expected_output, format, arg) \
  EXPECT_EQ(expected_output, fmt::sprintf(format, arg)) \
    << "format: " << format; \
  EXPECT_EQ(expected_output, fmt::sprintf(make_positional(format), arg))

TEST(PrintfTest, NoArgs) {
  EXPECT_EQ("test", fmt::sprintf("test"));
}

TEST(PrintfTest, Escape) {
  EXPECT_EQ("%", fmt::sprintf("%%"));
  EXPECT_EQ("before %", fmt::sprintf("before %%"));
  EXPECT_EQ("% after", fmt::sprintf("%% after"));
  EXPECT_EQ("before % after", fmt::sprintf("before %% after"));
  EXPECT_EQ("%s", fmt::sprintf("%%s"));
}

TEST(PrintfTest, PositionalArgs) {
  EXPECT_EQ("42", fmt::sprintf("%1$d", 42));
  EXPECT_EQ("before 42", fmt::sprintf("before %1$d", 42));
  EXPECT_EQ("42 after", fmt::sprintf("%1$d after",42));
  EXPECT_EQ("before 42 after", fmt::sprintf("before %1$d after", 42));
  EXPECT_EQ("answer = 42", fmt::sprintf("%1$s = %2$d", "answer", 42));
  EXPECT_EQ("42 is the answer",
      fmt::sprintf("%2$d is the %1$s", "answer", 42));
  EXPECT_EQ("abracadabra", fmt::sprintf("%1$s%2$s%1$s", "abra", "cad"));
}

TEST(PrintfTest, AutomaticArgIndexing) {
  EXPECT_EQ("abc", fmt::sprintf("%c%c%c", 'a', 'b', 'c'));
}

TEST(PrintfTest, NumberIsTooBigInArgIndex) {
  EXPECT_THROW_MSG(fmt::sprintf(format("%{}$", BIG_NUM)),
      FormatError, "invalid format string");
  EXPECT_THROW_MSG(fmt::sprintf(format("%{}$d", BIG_NUM)),
      FormatError, "number is too big in format");
}

TEST(PrintfTest, SwitchArgIndexing) {
  EXPECT_THROW_MSG(fmt::sprintf("%1$d%", 1, 2),
      FormatError, "invalid format string");
  EXPECT_THROW_MSG(fmt::sprintf(format("%1$d%{}d", BIG_NUM), 1, 2),
      FormatError, "number is too big in format");
  EXPECT_THROW_MSG(fmt::sprintf("%1$d%d", 1, 2),
      FormatError, "cannot switch from manual to automatic argument indexing");

  EXPECT_THROW_MSG(fmt::sprintf("%d%1$", 1, 2),
      FormatError, "invalid format string");
  EXPECT_THROW_MSG(fmt::sprintf(format("%d%{}$d", BIG_NUM), 1, 2),
      FormatError, "number is too big in format");
  EXPECT_THROW_MSG(fmt::sprintf("%d%1$d", 1, 2),
      FormatError, "cannot switch from automatic to manual argument indexing");

  // Indexing errors override width errors.
  EXPECT_THROW_MSG(fmt::sprintf(format("%d%1${}d", BIG_NUM), 1, 2),
      FormatError, "number is too big in format");
  EXPECT_THROW_MSG(fmt::sprintf(format("%1$d%{}d", BIG_NUM), 1, 2),
      FormatError, "number is too big in format");
}

TEST(PrintfTest, InvalidArgIndex) {
  EXPECT_THROW_MSG(fmt::sprintf("%0$d", 42), FormatError,
      "argument index is out of range in format");
  EXPECT_THROW_MSG(fmt::sprintf("%2$d", 42), FormatError,
      "argument index is out of range in format");
  EXPECT_THROW_MSG(fmt::sprintf(format("%{}$d", INT_MAX), 42),
      FormatError, "argument index is out of range in format");

  EXPECT_THROW_MSG(fmt::sprintf("%2$", 42),
      FormatError, "invalid format string");
  EXPECT_THROW_MSG(fmt::sprintf(format("%{}$d", BIG_NUM), 42),
      FormatError, "number is too big in format");
}

TEST(PrintfTest, DefaultAlignRight) {
  EXPECT_PRINTF("   42", "%5d", 42);
  EXPECT_PRINTF("  abc", "%5s", "abc");
}

TEST(PrintfTest, ZeroFlag) {
  EXPECT_PRINTF("00042", "%05d", 42);
  EXPECT_PRINTF("-0042", "%05d", -42);

  EXPECT_PRINTF("00042", "%05d", 42);
  EXPECT_PRINTF("-0042", "%05d", -42);
  EXPECT_PRINTF("-004.2", "%06g", -4.2);

  EXPECT_PRINTF("+00042", "%00+6d", 42);

  // '0' flag is ignored for non-numeric types.
  EXPECT_PRINTF("    x", "%05c", 'x');
}

TEST(PrintfTest, PlusFlag) {
  EXPECT_PRINTF("+42", "%+d", 42);
  EXPECT_PRINTF("-42", "%+d", -42);
  EXPECT_PRINTF("+0042", "%+05d", 42);
  EXPECT_PRINTF("+0042", "%0++5d", 42);

  // '+' flag is ignored for non-numeric types.
  EXPECT_PRINTF("x", "%+c", 'x');
}

TEST(PrintfTest, MinusFlag) {
  EXPECT_PRINTF("abc  ", "%-5s", "abc");
  EXPECT_PRINTF("abc  ", "%0--5s", "abc");
}

TEST(PrintfTest, SpaceFlag) {
  EXPECT_PRINTF(" 42", "% d", 42);
  EXPECT_PRINTF("-42", "% d", -42);
  EXPECT_PRINTF(" 0042", "% 05d", 42);
  EXPECT_PRINTF(" 0042", "%0  5d", 42);

  // ' ' flag is ignored for non-numeric types.
  EXPECT_PRINTF("x", "% c", 'x');
}

TEST(PrintfTest, HashFlag) {
  EXPECT_PRINTF("042", "%#o", 042);
  EXPECT_PRINTF("-042", "%#o", -042);
  EXPECT_PRINTF("0", "%#o", 0);

  EXPECT_PRINTF("0x42", "%#x", 0x42);
  EXPECT_PRINTF("0X42", "%#X", 0x42);
  EXPECT_PRINTF("-0x42", "%#x", -0x42);
  EXPECT_PRINTF("0", "%#x", 0);

  EXPECT_PRINTF("0x0042", "%#06x", 0x42);
  EXPECT_PRINTF("0x0042", "%0##6x", 0x42);

  EXPECT_PRINTF("-42.000000", "%#f", -42.0);
  EXPECT_PRINTF("-42.000000", "%#F", -42.0);

  char buffer[BUFFER_SIZE];
  safe_sprintf(buffer, "%#e", -42.0);
  EXPECT_PRINTF(buffer, "%#e", -42.0);
  safe_sprintf(buffer, "%#E", -42.0);
  EXPECT_PRINTF(buffer, "%#E", -42.0);

  EXPECT_PRINTF("-42.0000", "%#g", -42.0);
  EXPECT_PRINTF("-42.0000", "%#G", -42.0);

  safe_sprintf(buffer, "%#a", 16.0);
  EXPECT_PRINTF(buffer, "%#a", 16.0);
  safe_sprintf(buffer, "%#A", 16.0);
  EXPECT_PRINTF(buffer, "%#A", 16.0);

  // '#' flag is ignored for non-numeric types.
  EXPECT_PRINTF("x", "%#c", 'x');
}

TEST(PrintfTest, Width) {
  EXPECT_PRINTF("  abc", "%5s", "abc");

  // Width cannot be specified twice.
  EXPECT_THROW_MSG(fmt::sprintf("%5-5d", 42), FormatError,
      "unknown format code '-' for integer");

  EXPECT_THROW_MSG(fmt::sprintf(format("%{}d", BIG_NUM), 42),
      FormatError, "number is too big in format");
  EXPECT_THROW_MSG(fmt::sprintf(format("%1${}d", BIG_NUM), 42),
      FormatError, "number is too big in format");
}

TEST(PrintfTest, DynamicWidth) {
  EXPECT_EQ("   42", fmt::sprintf("%*d", 5, 42));
  EXPECT_EQ("42   ", fmt::sprintf("%*d", -5, 42));
  EXPECT_THROW_MSG(fmt::sprintf("%*d", 5.0, 42), FormatError,
      "width is not integer");
  EXPECT_THROW_MSG(fmt::sprintf("%*d"), FormatError,
      "argument index is out of range in format");
  EXPECT_THROW_MSG(fmt::sprintf("%*d", BIG_NUM, 42), FormatError,
      "number is too big in format");
}

TEST(PrintfTest, IntPrecision) {
  EXPECT_PRINTF("00042", "%.5d", 42);
  EXPECT_PRINTF("-00042", "%.5d", -42);
  EXPECT_PRINTF("00042", "%.5x", 0x42);
  EXPECT_PRINTF("0x00042", "%#.5x", 0x42);
  EXPECT_PRINTF("00042", "%.5o", 042);
  EXPECT_PRINTF("00042", "%#.5o", 042);

  EXPECT_PRINTF("  00042", "%7.5d", 42);
  EXPECT_PRINTF("  00042", "%7.5x", 0x42);
  EXPECT_PRINTF("   0x00042", "%#10.5x", 0x42);
  EXPECT_PRINTF("  00042", "%7.5o", 042);
  EXPECT_PRINTF("     00042", "%#10.5o", 042);

  EXPECT_PRINTF("00042  ", "%-7.5d", 42);
  EXPECT_PRINTF("00042  ", "%-7.5x", 0x42);
  EXPECT_PRINTF("0x00042   ", "%-#10.5x", 0x42);
  EXPECT_PRINTF("00042  ", "%-7.5o", 042);
  EXPECT_PRINTF("00042     ", "%-#10.5o", 042);
}

TEST(PrintfTest, FloatPrecision) {
  char buffer[BUFFER_SIZE];
  safe_sprintf(buffer, "%.3e", 1234.5678);
  EXPECT_PRINTF(buffer, "%.3e", 1234.5678);
  EXPECT_PRINTF("1234.568", "%.3f", 1234.5678);
  safe_sprintf(buffer, "%.3g", 1234.5678);
  EXPECT_PRINTF(buffer, "%.3g", 1234.5678);
  safe_sprintf(buffer, "%.3a", 1234.5678);
  EXPECT_PRINTF(buffer, "%.3a", 1234.5678);
}

TEST(PrintfTest, IgnorePrecisionForNonNumericArg) {
  EXPECT_PRINTF("abc", "%.5s", "abc");
}

TEST(PrintfTest, DynamicPrecision) {
  EXPECT_EQ("00042", fmt::sprintf("%.*d", 5, 42));
  EXPECT_EQ("42", fmt::sprintf("%.*d", -5, 42));
  EXPECT_THROW_MSG(fmt::sprintf("%.*d", 5.0, 42), FormatError,
      "precision is not integer");
  EXPECT_THROW_MSG(fmt::sprintf("%.*d"), FormatError,
      "argument index is out of range in format");
  EXPECT_THROW_MSG(fmt::sprintf("%.*d", BIG_NUM, 42), FormatError,
      "number is too big in format");
  if (sizeof(fmt::LongLong) != sizeof(int)) {
    fmt::LongLong prec = static_cast<fmt::LongLong>(INT_MIN) - 1;
    EXPECT_THROW_MSG(fmt::sprintf("%.*d", prec, 42), FormatError,
        "number is too big in format");
 }
}

bool IsSupported(const std::string &format) {
#if _MSC_VER
  // MSVC doesn't support hh, j, z and t format specifiers.
  return format.substr(1, 2) != "hh";
#else
  return true;
#endif
}

template <typename T>
struct MakeSigned { typedef T Type; };

#define SPECIALIZE_MAKE_SIGNED(T, S) \
  template <> \
  struct MakeSigned<T> { typedef S Type; }

SPECIALIZE_MAKE_SIGNED(char, signed char);
SPECIALIZE_MAKE_SIGNED(unsigned char, signed char);
SPECIALIZE_MAKE_SIGNED(unsigned short, short);
SPECIALIZE_MAKE_SIGNED(unsigned, int);
SPECIALIZE_MAKE_SIGNED(unsigned long, long);
SPECIALIZE_MAKE_SIGNED(fmt::ULongLong, fmt::LongLong);

template <typename T, typename U>
std::string sprintf_int(std::string format, U value) {
  char buffer[BUFFER_SIZE];
  char type = format[format.size() - 1];
  if (type == 'd' || type == 'i') {
    typedef typename MakeSigned<T>::Type Signed;
    safe_sprintf(buffer, format.c_str(), static_cast<Signed>(value));
  } else {
    typedef typename fmt::internal::MakeUnsigned<T>::Type Unsigned;
    safe_sprintf(buffer, format.c_str(), static_cast<Unsigned>(value));
  }
  return buffer;
}

#define EXPECT_STD_PRINTF(format, T, arg) { \
  char buffer[BUFFER_SIZE]; \
  if (IsSupported(format)) { \
    safe_sprintf(buffer, fmt::StringRef(format).c_str(), arg); \
    EXPECT_PRINTF(buffer, format, arg); \
  } \
  EXPECT_PRINTF(sprintf_int<T>(format, arg), format, arg); \
}

template <typename T>
void TestLength(const char *length_spec) {
  T min = std::numeric_limits<T>::min(), max = std::numeric_limits<T>::max();
  const char types[] = {'d', 'i', 'u', 'o', 'x', 'X'};
  for (int i = 0; i < sizeof(types); ++i) {
    std::string format = fmt::format("%{}{}", length_spec, types[i]);
    EXPECT_STD_PRINTF(format, T, 42);
    EXPECT_STD_PRINTF(format, T, min);
    EXPECT_STD_PRINTF(format, T, max);
    EXPECT_STD_PRINTF(format, T, fmt::LongLong(min) - 1);
    EXPECT_STD_PRINTF(format, T, fmt::LongLong(max) + 1);
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<short>::min());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<unsigned short>::max());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<int>::min());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<int>::max());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<unsigned>::min());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<unsigned>::max());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<fmt::LongLong>::min());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<fmt::LongLong>::max());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<fmt::ULongLong>::min());
    EXPECT_STD_PRINTF(format, T, std::numeric_limits<fmt::ULongLong>::max());
  }
}

TEST(PrintfTest, Length) {
  EXPECT_EQ("-1", sprintf_int<unsigned char>("%hhd", UCHAR_MAX));
  EXPECT_EQ("255", sprintf_int<unsigned char>("%hhu", UCHAR_MAX));
  TestLength<signed char>("hh");
  TestLength<unsigned char>("hh");
  TestLength<short>("h");
  TestLength<unsigned short>("h");
  TestLength<long>("l");
  TestLength<unsigned long>("l");
  // TODO: more tests
}

// TODO: test type specifier
