/*
 Tests of variadic function emulation macros.

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

#include <utility>
#include <gtest/gtest.h>
#include "format.h"

#define IDENTITY(x) x

TEST(UtilTest, Gen) {
  int values[] = {FMT_GEN(10, IDENTITY)};
  for (int i = 0; i < 10; ++i)
    EXPECT_EQ(i, values[i]);
}

#define MAKE_PAIR(x, y) std::make_pair(x, y)

TEST(UtilTest, ForEach) {
  std::pair<char, int> values[] = {
      FMT_FOR_EACH10(MAKE_PAIR, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j')
  };
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ('a' + i, values[i].first);
    EXPECT_EQ(i, values[i].second);
  }
}

int result;

#define MAKE_TEST(func) \
  void func(const char *format, const fmt::ArgList &args) { \
    result = 0; \
    for (std::size_t i = 0, n = args.size(); i < n; ++i) \
      result += args[i].int_value; \
  }

MAKE_TEST(TestFunc)
FMT_WRAP1(TestFunc, const char *, 1)

TEST(UtilTest, Wrap1) {
  result = 0;
  TestFunc("", 42);
  EXPECT_EQ(42, result);
}

MAKE_TEST(TestVariadicVoid)
FMT_VARIADIC_VOID(TestVariadicVoid, const char *)

TEST(UtilTest, VariadicVoid) {
  result = 0;
  TestVariadicVoid("", 10, 20, 30, 40, 50, 60, 70, 80, 90, 100);
  EXPECT_EQ(550, result);
}

template <int>
struct S {};

#define GET_TYPE(n) S<n>

int TestVariadic(FMT_GEN(10, GET_TYPE), const fmt::ArgList &args) { \
  int result = 0; \
  for (std::size_t i = 0, n = args.size(); i < n; ++i) \
    result += args[i].int_value; \
  return result;
}
FMT_VARIADIC(int, TestVariadic, FMT_GEN(10, GET_TYPE))

#define MAKE_ARG(n) S<n>()

TEST(UtilTest, Variadic) {
  EXPECT_EQ(550, TestVariadic(FMT_GEN(10, MAKE_ARG),
      10, 20, 30, 40, 50, 60, 70, 80, 90, 100));
}
