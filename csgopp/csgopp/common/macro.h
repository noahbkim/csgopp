#pragma once

#include <cstdio>

#include "../error.h"

#define LOG(FMT, ...) fprintf(stderr, "[%s:%d] " FMT "\n", __FILE__, __LINE__, __VA_ARGS__);

#define ASSERT(CONDITION, FMT, ...) do \
{ \
    if (!(CONDITION)) \
    { \
        LOG(FMT, __VA_ARGS__) \
        throw csgopp::error::GameError("failed assertion " #CONDITION); \
    } \
} while (false)

#define OK(SUCCESS) ASSERT(SUCCESS, #SUCCESS)
