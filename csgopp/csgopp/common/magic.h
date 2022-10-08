#pragma once

#define SFINAE(T, RETURN) { static_assert(sizeof(T) == 0); return RETURN; }
