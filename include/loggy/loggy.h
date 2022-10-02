#ifndef LOGGY_LOGGY_H
#define LOGGY_LOGGY_H

#define ANSI_BLACK      "\033[30m"
#define ANSI_RED        "\033[31m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_ORANGE     "\033[33m"
#define ANSI_BLUE       "\033[34m"
#define ANSI_PURPLE     "\033[35m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_LIGHT_GRAY "\033[37m"
#define ANSI_NONE       "\033[0m"

typedef enum LgLevel_ {
    LG_LEVEL_OFF = 0,
    LG_LEVEL_INFO, LG_LEVEL_WARN,
    LG_LEVEL_ERROR, LG_LEVEL_FATAL
} LgLevel;

typedef struct LgLogger_ {
    char* format;

    int stdout_is_terminal;
    int stdout_supports_colors;
    
    int stderr_is_terminal;
    int stderr_supports_colors;

    int should_quit;
} LgLogger;

void loggy_init(LgLogger* logger, char* format);
void loggy_log(LgLogger* logger, LgLevel level,
               const char* file, int line, const char* fmt, ...);

#define LG_INFO(x, ...)  loggy_log(x, LG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LG_WARN(x, ...)  loggy_log(x, LG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LG_ERROR(x, ...) loggy_log(x, LG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LG_FATAL(x, ...) loggy_log(x, LG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif // LOGGY_LOGGY_h
