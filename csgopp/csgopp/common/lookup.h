#pragma once

#include <type_traits>

#define LOOKUP(NAME, KEY_TYPE, VALUE_TYPE, CASES) \
constexpr VALUE_TYPE NAME(KEY_TYPE key) {         \
  switch (key) {                                  \
    using _key_type = KEY_TYPE;                   \
    using _value_type = VALUE_TYPE; \
    CASES \
  } \
}

#define ENUM(KEY) case static_cast<typename std::underlying_type<_value_type>::type>(KEY): return KEY;
#define CASE(KEY, VALUE) case KEY: return VALUE;
#define DEFAULT(THEN) default: THEN;
