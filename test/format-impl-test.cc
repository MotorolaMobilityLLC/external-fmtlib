/*
 Formatting library implementation tests.

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

#define FMT_NOEXCEPT
#include "test-assert.h"

// Include format.cc instead of format.h to test implementation-specific stuff.
#include "fmt/format.cc"
#include "fmt/printf.h"

#include <algorithm>
#include <cstring>

#include "gmock/gmock.h"
#include "gtest-extra.h"
#include "util.h"

#undef min
#undef max

template <typename T>
struct ValueExtractor {
  T operator()(T value) {
    return value;
  }

  template <typename U>
  T operator()(U) {
    throw std::runtime_error(fmt::format("invalid type {}", typeid(U).name()));
    return T();
  }
};

TEST(FormatTest, ArgConverter) {
  long long value = std::numeric_limits<long long>::max();
  auto arg = fmt::internal::make_arg<fmt::context>(value);
  visit(fmt::internal::ArgConverter<long long, fmt::context>(arg, 'd'), arg);
  EXPECT_EQ(value, visit(ValueExtractor<long long>(), arg));
}

TEST(FormatTest, FormatNegativeNaN) {
  double nan = std::numeric_limits<double>::quiet_NaN();
  if (fmt::internal::fputil::isnegative(-nan))
    EXPECT_EQ("-nan", fmt::format("{}", -nan));
  else
    fmt::print("Warning: compiler doesn't handle negative NaN correctly");
}

TEST(FormatTest, StrError) {
  char *message = 0;
  char buffer[BUFFER_SIZE];
  EXPECT_ASSERT(fmt::safe_strerror(EDOM, message = 0, 0), "invalid buffer");
  EXPECT_ASSERT(fmt::safe_strerror(EDOM, message = buffer, 0),
                "invalid buffer");
  buffer[0] = 'x';
#if defined(_GNU_SOURCE) && !defined(__COVERITY__)
  // Use invalid error code to make sure that safe_strerror returns an error
  // message in the buffer rather than a pointer to a static string.
  int error_code = -1;
#else
  int error_code = EDOM;
#endif

  int result = fmt::safe_strerror(error_code, message = buffer, BUFFER_SIZE);
  EXPECT_EQ(0, result);
  std::size_t message_size = std::strlen(message);
  EXPECT_GE(BUFFER_SIZE - 1u, message_size);
  EXPECT_EQ(get_system_error(error_code), message);

  // safe_strerror never uses buffer on MinGW.
#ifndef __MINGW32__
  result = fmt::safe_strerror(error_code, message = buffer, message_size);
  EXPECT_EQ(ERANGE, result);
  result = fmt::safe_strerror(error_code, message = buffer, 1);
  EXPECT_EQ(buffer, message);  // Message should point to buffer.
  EXPECT_EQ(ERANGE, result);
  EXPECT_STREQ("", message);
#endif
}

TEST(FormatTest, FormatErrorCode) {
  std::string msg = "error 42", sep = ": ";
  {
    fmt::memory_buffer buffer;
    format_to(buffer, "garbage");
    fmt::format_error_code(buffer, 42, "test");
    EXPECT_EQ("test: " + msg, to_string(buffer));
  }
  {
    fmt::memory_buffer buffer;
    std::string prefix(
        fmt::internal::INLINE_BUFFER_SIZE - msg.size() - sep.size() + 1, 'x');
    fmt::format_error_code(buffer, 42, prefix);
    EXPECT_EQ(msg, to_string(buffer));
  }
  int codes[] = {42, -1};
  for (std::size_t i = 0, n = sizeof(codes) / sizeof(*codes); i < n; ++i) {
    // Test maximum buffer size.
    msg = fmt::format("error {}", codes[i]);
    fmt::memory_buffer buffer;
    std::string prefix(
        fmt::internal::INLINE_BUFFER_SIZE - msg.size() - sep.size(), 'x');
    fmt::format_error_code(buffer, codes[i], prefix);
    EXPECT_EQ(prefix + sep + msg, to_string(buffer));
    std::size_t size = fmt::internal::INLINE_BUFFER_SIZE;
    EXPECT_EQ(size, buffer.size());
    buffer.resize(0);
    // Test with a message that doesn't fit into the buffer.
    prefix += 'x';
    fmt::format_error_code(buffer, codes[i], prefix);
    EXPECT_EQ(msg, to_string(buffer));
  }
}
