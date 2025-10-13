#pragma once

#include <time.h>
#include <type_traits>

// ServerLua: use a 64-bit time if we're on a platform with 32-bit times.
using lua_time_t = std::conditional<sizeof(time_t) != sizeof(int32_t), time_t, int64_t>::type;

struct tm * localtime64_r_y2038(const lua_time_t *time, struct tm *local_tm);
struct tm * gmtime64_r_y2038(const lua_time_t *in_time, struct tm *p);
