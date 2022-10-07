#ifndef FINCH_CORE_H
#define FINCH_CORE_H

#include <stdint.h>
#include <stdbool.h>
#include <loggy/loggy.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

typedef bool b32;

LgLogger* fc_get_engine_logger();
LgLogger* fc_get_application_logger();

#define FC_ENGINE_INFO(...)  loggy_log(fc_get_engine_logger(), \
                                     LG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_WARN(...)  loggy_log(fc_get_engine_logger(), \
                                     LG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_ERROR(...) loggy_log(fc_get_engine_logger(), \
                                     LG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_FATAL(...) loggy_log(fc_get_engine_logger(), \
                                     LG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define FC_INFO(...)  loggy_log(fc_get_application_logger(), \
                                LG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define FC_WARN(...)  loggy_log(fc_get_application_logger(), \
                                LG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ERROR(...) loggy_log(fc_get_application_logger(), \
                                LG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define FC_FATAL(...) loggy_log(fc_get_application_logger(), \
                                LG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif // FINCH_CORE_H
