#pragma once

#define CASE(KEY, VALUE) case KEY: return VALUE;
#define LOOKUP(NAME, KEY_TYPE, VALUE_TYPE, CASES) \
constexpr VALUE_TYPE NAME(KEY_TYPE key) { \
  switch (key) { \
    CASES \
  } \
}
#define DEFAULT(THEN) default: THEN;
