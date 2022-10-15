#include "finch/log/log.h"
#include "finch/core/core.h"
#include "finch/platform/platform.h"
#include "finch/utils/string.h"

#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

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
    platform_set_terminal_color(terminal_colors[level]);
    
    char buf[1024] = {0};
                
    time_t     now;
    struct tm  ts;
    time(&now);
    
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%T", &ts);    
    u32 buf_len = string_length_null_terminated(buf);
    
    buf_len = buf_len + string_format(buf + buf_len, sizeof(buf) - buf_len,
                            " (%s:%u) %s (%s) ", file, line, name, levels[level]);
    string_copy(buf + buf_len, msg);
    
    platform_write_to_stdout(buf);
    platform_write_to_stdout("\n");
    
    platform_set_terminal_color(FC_TERM_COLOR_WHITE);
}
