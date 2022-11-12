#include "finch/utils/log.h"
#include "finch/core/core.h"
#include "finch/platform/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static char* levels[] = {"OFF", "TRACE", "INFO", "WARN", "ERROR"};

void fc_logger_log(char* name, FcLogLevel level,
                   char* file, u32 line, char* msg)
{
    FcTerminalColor terminal_colors[] = {
        FC_TERM_COLOR_WHITE,
        FC_TERM_COLOR_WHITE,
        FC_TERM_COLOR_GREEN,
        FC_TERM_COLOR_ORANGE,
        FC_TERM_COLOR_RED
    };
    char buf[1024] = {0};

    time_t     now;
    struct tm  ts;
    time(&now);

    ts = *localtime(&now);
    platform_set_terminal_color(terminal_colors[level]);
    strftime(buf, sizeof(buf), "%T", &ts);
    printf("%s (%s:%u) %s [%s] %s\n", buf, file, line, name, levels[level], msg);
    platform_set_terminal_color(FC_TERM_COLOR_WHITE);
}
