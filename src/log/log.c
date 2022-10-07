#include "log/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
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

static int32_t determine_color_support()
{
    int fd[2];
    if (pipe(fd) < 0) {
        return 0;
    };

    pid_t pid = fork();
    if (pid < 0) {
        return 0;
    }
    
    if (pid == 0) {
        // Child
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            return 0;
        }
        close(fd[1]);
        if (execl("/bin/tput", "/bin/tput", "colors", NULL) < 0) {
            return 0;
        }
    }

    close(fd[1]);
    char buf[100];
    while (read(fd[0], buf, sizeof(buf)) != 0) {};
    return atoi(buf);    
}

void fc_logger_init(FcLogger* logger, char* format)
{
    int colors = determine_color_support();
    
    logger->stdout_is_terminal = isatty(STDOUT_FILENO);
    logger->stderr_is_terminal = isatty(STDERR_FILENO);
    
    logger->stdout_supports_colors = (colors == 256 && logger->stdout_is_terminal);
    logger->stderr_supports_colors = (colors == 256 && logger->stderr_is_terminal);

    logger->format = format;
}

static void format_msg(char* dest, FcLogger* logger, FcLogLevel level,
                       const char* file, int line,
                       const char* fmt, va_list args)
{
    char temp[1000] = {0};

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
                                    sprintf(dest + j, ANSI_GREEN);
                                    j += strlen(ANSI_GREEN);
                                }
                            } break;
                            case LG_LEVEL_WARN: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_ORANGE);
                                    j += strlen(ANSI_ORANGE);
                                }
                            } break;
                            case LG_LEVEL_ERROR: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_RED);
                                    j += strlen(ANSI_RED);
                                }
                            } break;
                            case LG_LEVEL_FATAL: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_RED);
                                    j += strlen(ANSI_RED);
                                }
                            } break;
                            default: {}
                        }
                        strcpy(dest + j, levels[level]);
                        j += strlen(levels[level]);
                        switch (level) {
                            case LG_LEVEL_INFO: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_NONE);
                                    j += strlen(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_WARN: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_NONE);
                                    j += strlen(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_ERROR: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_NONE);
                                    j += strlen(ANSI_NONE);
                                }
                            } break;
                            case LG_LEVEL_FATAL: {
                                if (logger->stdout_supports_colors) {
                                    sprintf(dest + j, ANSI_NONE);
                                    j += strlen(ANSI_NONE);
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
                            sprintf(dest + j, ANSI_BLACK);
                            j += strlen(ANSI_BLACK);
                        } break;
                        case 'r': {
                            sprintf(dest + j, ANSI_RED);
                            j += strlen(ANSI_RED);
                        } break;
                        case 'g': {
                            sprintf(dest + j, ANSI_GREEN);
                            j += strlen(ANSI_GREEN);
                        } break;
                        case 'o': {
                            sprintf(dest + j, ANSI_ORANGE);
                            j += strlen(ANSI_ORANGE);
                        } break;
                        case 'b': {
                            sprintf(dest + j, ANSI_BLUE);
                            j += strlen(ANSI_BLUE);
                        } break;
                        case 'p': {
                            sprintf(dest + j, ANSI_PURPLE);
                            j += strlen(ANSI_PURPLE);
                        } break;
                        case 'c': {
                            sprintf(dest + j, ANSI_CYAN);
                            j += strlen(ANSI_CYAN);
                        } break;
                        case 'l': {
                            sprintf(dest + j, ANSI_LIGHT_GRAY);
                            j += strlen(ANSI_LIGHT_GRAY);
                        } break;
                        case 'n': {
                            sprintf(dest + j, ANSI_NONE);
                            j += strlen(ANSI_NONE);
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
                        strcpy(dest + j, file);
                        j += strlen(file);
                    } break;
                            
                        // Line number
                    case 'l': {
                        sprintf(temp, "%d", line);
                        strcpy(dest + j, temp);
                        j += strlen(temp);
                    } break;
                    default: {
                        fprintf(stderr, "Unrecognized format option '%%%c'", thing);
                    }
                }
            } break;

            // Time related, let strftime handle it
            case 'T': {
                i += 1;
                char buf[1024];
                sprintf(buf, "%%%c", fmt[i]);
                    
                time_t     now;
                struct tm  ts;
                time(&now);

                ts = *localtime(&now);
                strftime(temp, sizeof(temp), buf, &ts);
                strcpy(dest + j, temp);
                j += strlen(temp);
            } break;

            // Varargs, let printf handle it
            case 'V': {
                i += 1;
                char thing = fmt[i];
                char buf[1024];
                sprintf(buf, "%%%c", fmt[i]);

                switch (thing) {
                    case 'd': {
                        sprintf(temp, buf, va_arg(args, int));
                    } break;
                    case 'u': {
                        sprintf(temp, buf, va_arg(args, unsigned int));
                    } break;
                    case 'f': {
                        sprintf(temp, buf, va_arg(args, double));
                    } break;
                    case 'c': {
                        sprintf(temp, buf, va_arg(args, int));
                    } break;
                    case 's': {
                        sprintf(temp, buf, va_arg(args, char*));
                    } break;
                    case 'p': {
                        sprintf(temp, buf, va_arg(args, int64_t));
                    } break;
                }

                strcpy(dest + j, temp);
                j += strlen(temp);
            }
        }
        i += 1;
    }
}

void fc_logger_log(FcLogger* logger, FcLogLevel level,
                   const char* file, int line, const char* fmt, ...)
{
    char buf[1000] = {0};

    va_list args;
    va_start(args, fmt);
    format_msg(buf, logger, level, file, line, logger->format, args);
    format_msg(buf + strlen(buf), logger, level, file, line, fmt, args);
    va_end(args);
    
    printf("%s\n", buf);
    
    if (logger->should_quit) {
        exit(EXIT_FAILURE);
    }
}
