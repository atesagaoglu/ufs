#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

void logger(log_level_t level, const char *format, ...) {
    va_list va;
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[256];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%H:%M:%S - %m.%d.%Y", timeinfo);

    va_start(va, format);

    switch (level) {
    case L_INFO:
        printf("\033[32m[INFO - %s]\033[0m ", buffer);
        break;
    case L_WARN:
        printf("\033[33m[WARN - %s]\033[0m ", buffer);
        break;
    case L_ERROR:
        printf("\033[31m[ERROR - %s]\033[0m ", buffer);
        break;
    default:
        break;
    }

    vprintf(format, va);
    va_end(va);
}
