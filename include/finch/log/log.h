#ifndef FINCH_LOG_LOG_H
#define FINCH_LOG_LOG_H

#define ANSI_BLACK      "\033[30m"
#define ANSI_RED        "\033[31m"
#define ANSI_GREEN      "\033[32m"
#define ANSI_ORANGE     "\033[33m"
#define ANSI_BLUE       "\033[34m"
#define ANSI_PURPLE     "\033[35m"
#define ANSI_CYAN       "\033[36m"
#define ANSI_LIGHT_GRAY "\033[37m"
#define ANSI_NONE       "\033[0m"

typedef enum FcLogLevel_ {
    LG_LEVEL_OFF = 0,
    LG_LEVEL_INFO, LG_LEVEL_WARN,
    LG_LEVEL_ERROR, LG_LEVEL_FATAL
} FcLogLevel;

typedef struct FcLogger_ {
    char* format;

    int stdout_is_terminal;
    int stdout_supports_colors;
    
    int stderr_is_terminal;
    int stderr_supports_colors;

    int should_quit;
} FcLogger;

void fc_logger_init(FcLogger* logger, char* format);
void fc_logger_log(FcLogger* logger, FcLogLevel level,
                   const char* file, int line, const char* fmt, ...);

FcLogger* fc_get_engine_logger();
FcLogger* fc_get_application_logger();

#define FC_ENGINE_INFO(...)  fc_logger_log(fc_get_engine_logger(), \
                                           LG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_WARN(...)  fc_logger_log(fc_get_engine_logger(), \
                                           LG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_ERROR(...) fc_logger_log(fc_get_engine_logger(), \
                                           LG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ENGINE_FATAL(...) fc_logger_log(fc_get_engine_logger(), \
                                           LG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define FC_INFO(...)  fc_logger_log(fc_get_application_logger(), \
                                    LG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define FC_WARN(...)  fc_logger_log(fc_get_application_logger(), \
                                    LG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define FC_ERROR(...) fc_logger_log(fc_get_application_logger(), \
                                    LG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define FC_FATAL(...) fc_logger_log(fc_get_application_logger(), \
                                    LG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif // FINCH_LOG_LOG_H
