#include "finch/log/log.h"
#include "finch/core/core.h"
#include "finch/core/utils.h"
#include "finch/platform/platform.h"

#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

static FcLogger global_engine_logger;
static FcLogger global_application_logger;

FcLogger* fc_get_engine_logger()
{
    return &global_engine_logger;
}

FcLogger* fc_get_application_logger()
{
    return &global_application_logger;
}

static char* levels[] = {"OFF", "INFO", "WARN", "ERROR", "FATAL"};

void fc_logger_init(FcLogger* logger, char* format)
{
    b32 terminal_supports_colors = platform_terminal_supports_colors();
    
    logger->stdout_is_terminal = platform_stdout_is_terminal();
    logger->stderr_is_terminal = platform_stderr_is_terminal();
    
    logger->stdout_supports_colors = (terminal_supports_colors && logger->stdout_is_terminal);
    logger->stderr_supports_colors = (terminal_supports_colors && logger->stderr_is_terminal);

    logger->format = format;
}

static void format_msg(char* dest, FcLogger* logger, FcLogLevel level,
                       char* file, u32 line,
                       const char* fmt, va_list args)
{
    char temp[1024] = {0};

    int i = 0;
    int j = 0;
    while (fmt && fmt[i]) {
        if (fmt[i] != '%') {
            dest[j++] = fmt[i++];
            continue;
        }

        if (fmt[i+1] == '%') {
            dest[j++] = fmt[i++];
            i += 1;
            continue;
        }
        
        i += 1;
        switch (fmt[i]) {

            // General
            case 'G': {
                i += 1;
                char thing = fmt[i];
                switch (thing) {
                    case 'l': {
                        switch (level) {
                            case LG_LEVEL_INFO: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_GREEN);
                                    j += string_length_null_terminated(ANSI_GREEN);
                                }
                            } break;
                            case LG_LEVEL_WARN: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_ORANGE);
                                    j += string_length_null_terminated(ANSI_ORANGE);
                                }
                            } break;
                            case LG_LEVEL_ERROR: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_RED);
                                    j += string_length_null_terminated(ANSI_RED);
                                }
                            } break;
                            case LG_LEVEL_FATAL: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_RED);
                                    j += string_length_null_terminated(ANSI_RED);
                                }
                            } break;
                            default: {}
                        }
                        string_copy(dest + j, levels[level]);
                        j += string_length_null_terminated(levels[level]);
                        switch (level) {
                            case LG_LEVEL_INFO: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_NONE);
                                    j += string_length_null_terminated(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_WARN: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_NONE);
                                    j += string_length_null_terminated(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_ERROR: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_NONE);
                                    j += string_length_null_terminated(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_FATAL: {
                                if (logger->stdout_supports_colors) {
                                    string_copy(dest + j, ANSI_NONE);
                                    j += string_length_null_terminated(ANSI_NONE);
                                }
                            } break;
                            default: {}
                        }
                    } break;
                }
            } break;

            // Coloring
            case 'C': {
                i += 1;
                char thing = fmt[i];
                if (logger->stdout_supports_colors) {
                    switch (thing) {
                        case 'k': {
                            string_copy(dest + j, ANSI_BLACK);
                            j += string_length_null_terminated(ANSI_BLACK);
                        } break;
                        case 'r': {
                            string_copy(dest + j, ANSI_RED);
                            j += string_length_null_terminated(ANSI_RED);
                        } break;
                        case 'g': {
                            string_copy(dest + j, ANSI_GREEN);
                            j += string_length_null_terminated(ANSI_GREEN);
                        } break;
                        case 'o': {
                            string_copy(dest + j, ANSI_ORANGE);
                            j += string_length_null_terminated(ANSI_ORANGE);
                        } break;
                        case 'b': {
                            string_copy(dest + j, ANSI_BLUE);
                            j += string_length_null_terminated(ANSI_BLUE);
                        } break;
                        case 'p': {
                            string_copy(dest + j, ANSI_PURPLE);
                            j += string_length_null_terminated(ANSI_PURPLE);
                        } break;
                        case 'c': {
                            string_copy(dest + j, ANSI_CYAN);
                            j += string_length_null_terminated(ANSI_CYAN);
                        } break;
                        case 'l': {
                            string_copy(dest + j, ANSI_LIGHT_GRAY);
                            j += string_length_null_terminated(ANSI_LIGHT_GRAY);
                        } break;
                        case 'n': {
                            string_copy(dest + j, ANSI_NONE);
                            j += string_length_null_terminated(ANSI_NONE);
                        } break;
                    }   
                }
            } break;
                
            // File related
            case 'F': {
                i += 1;
                char thing = fmt[i];
                switch (thing) {
                    // File name
                    case 'n': {
                        string_copy(dest + j, file);
                        j += string_length_null_terminated(file);
                    } break;
                            
                        // Line number
                    case 'l': {
                        u64_to_null_terminated_string(line, temp, 1024, 10);
                        string_copy(dest + j, temp);
                        j += string_length_null_terminated(temp);
                    } break;
                    default: {
                        FC_ENGINE_WARN("Logger: Unrecognized format option '%%F%Vc'", thing);
                    }
                }
            } break;

            // Time related, let strftime handle it
            case 'T': {
                i += 1;
                char buf[3] = {'%', fmt[i], '\0'};
                
                time_t     now;
                struct tm  ts;
                time(&now);

                ts = *localtime(&now);
                strftime(temp, sizeof(temp), buf, &ts);
                string_copy(dest + j, temp);
                j += string_length_null_terminated(temp);
            } break;

            // Varargs
            case 'V': {
                i += 1;
                char thing = fmt[i];

                switch (thing) {
                    case 'd': {
                        s64_to_null_terminated_string(va_arg(args, s32),
                                                      temp, 1024, 10);
                    } break;
                    case 'u': {
                        u64_to_null_terminated_string(va_arg(args, u32),
                                                      temp, 1024, 10);
                    } break;
                    case 'x': {
                        u64_to_null_terminated_string(va_arg(args, u32),
                                                      temp, 1024, 16);
                    } break;
                    case 'b': {
                        u64_to_null_terminated_string(va_arg(args, u32),
                                                      temp, 1024, 2);
                    } break;
                    case 'f': {
                        f64_to_null_terminated_string(va_arg(args, f64),
                                                      temp, 1024, 5);
                    } break;
                    case 'c': {
                        temp[0] = (char)va_arg(args, s32);
                        temp[1] = '\0';
                    } break;
                    case 's': {
                        string_copy(temp, va_arg(args, char*));
                    } break;
                    case 'p': {
                        temp[0] = '0';
                        temp[1] = 'x';
                        u64_to_null_terminated_string(va_arg(args, u64),
                                                      temp + 2, 1024, 16);
                    } break;
                    default: {
                        FC_ENGINE_WARN("Logger: Unrecognized format option '%%V%Vc'", thing);
                    }
                }

                string_copy(dest + j, temp);
                j += string_length_null_terminated(temp);
            }
        }
        i += 1;
    }
}

void fc_logger_log(FcLogger* logger, FcLogLevel level,
                   char* file, int line, const char* fmt, ...)
{
    char buf[1000] = {0};

    va_list args;
    va_start(args, fmt);
    format_msg(buf, logger, level, file, line, logger->format, args);
    format_msg(buf + string_length_null_terminated(buf),
               logger, level, file, line, fmt, args);
    va_end(args);
    
    platform_write_to_stdout(buf);
    platform_write_to_stdout("\n");
    
    if (logger->should_quit) {
        exit(EXIT_FAILURE);
    }
}
