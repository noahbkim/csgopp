#pragma once

#include <cstdio>

#include "../error.h"

// Two levels ensures expansion
#define STR_(VALUE) #VALUE
#define STR(VALUE) STR_(VALUE)

#define WHERE() __FILE__ ":" STR(__LINE__)

#define LOG(FMT, ...) fprintf(stderr, WHERE() " " FMT "\n", ## __VA_ARGS__);

#define ASSERT(CONDITION, FMT, ...) do \
{ \
    if (!(CONDITION)) \
    { \
        LOG(FMT, ## __VA_ARGS__) \
        throw csgopp::error::GameError("failed assertion " #CONDITION); \
    } \
} while (false)

#define OK(SUCCESS) ASSERT(SUCCESS, #SUCCESS)
