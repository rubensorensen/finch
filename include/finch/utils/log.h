#include "finch/core/core.h"

#include <stdio.h>

#ifndef FINCH_LOG_LOG_H
#define FINCH_LOG_LOG_H

typedef enum _FcTerminalColor {
    FC_TERM_COLOR_WHITE,
    FC_TERM_COLOR_GREEN,
    FC_TERM_COLOR_ORANGE,
    FC_TERM_COLOR_RED
} FcTerminalColor;

typedef enum _FcLogLevel {
    FC_LOG_LEVEL_OFF = 0,
    FC_LOG_LEVEL_TRACE,
    FC_LOG_LEVEL_INFO,
    FC_LOG_LEVEL_WARN,
    FC_LOG_LEVEL_ERROR
} FcLogLevel;

void fc_logger_log(char* name, FcLogLevel level,
                   char* file, u32 line, char* msg);

#define FC_LOG(NAME, LEVEL, ...) {                                      \
        char FINCH_LOGBUF[1024];                                        \
        sprintf(FINCH_LOGBUF, __VA_ARGS__); \
        fc_logger_log(NAME, LEVEL,                                      \
                      __FILE__, __LINE__, FINCH_LOGBUF);                \
    }

#ifdef FINCH_LOGGING

#define FC_ENGINE_TRACE(...)  FC_LOG("FINCH", FC_LOG_LEVEL_TRACE, __VA_ARGS__)
#define FC_ENGINE_INFO(...)   FC_LOG("FINCH", FC_LOG_LEVEL_INFO, __VA_ARGS__)
#define FC_ENGINE_WARN(...)   FC_LOG("FINCH", FC_LOG_LEVEL_WARN, __VA_ARGS__)
#define FC_ENGINE_ERROR(...)  FC_LOG("FINCH", FC_LOG_LEVEL_ERROR, __VA_ARGS__)

#define FC_TRACE(...)  FC_LOG("APPLICATION", FC_LOG_LEVEL_TRACE, __VA_ARGS__)
#define FC_INFO(...)   FC_LOG("APPLICATION", FC_LOG_LEVEL_INFO, __VA_ARGS__)
#define FC_WARN(...)   FC_LOG("APPLICATION", FC_LOG_LEVEL_WARN, __VA_ARGS__)
#define FC_ERROR(...)  FC_LOG("APPLICATION", FC_LOG_LEVEL_ERROR, __VA_ARGS__)

#else

#define FC_ENGINE_TRACE(...)  
#define FC_ENGINE_INFO(...)   
#define FC_ENGINE_WARN(...)   
#define FC_ENGINE_ERROR(...)  

#define FC_TRACE(...)  
#define FC_INFO(...)   
#define FC_WARN(...)   
#define FC_ERROR(...)  

#endif // FINCH_LOGGING

#endif // FINCH_LOG_LOG_H
