#ifndef UFS_LOG_H
#define UFS_LOG_H

typedef enum {
    L_ERROR,
    L_WARN,
    L_INFO,
} log_level_t;

void logger(log_level_t level, const char *format, ...);

#endif
