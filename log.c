#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"
#define DEBUG 1

char log_created = 0;

void t_log (const char *format, ...)
{
    FILE *file;
    char log_msg[256];
    char log_str[256];
    int time_len = 0;
    va_list ap;
    
    memset(log_str, 0, 256);

    va_start(ap, format);
    vsprintf(log_str, format, ap);
#if DEBUG
    printf("%s\n", log_str);
#endif

    if (!log_created) {
        file = fopen(LOGFILE, "a");
        log_created = 1;
    }
    else
        file = fopen(LOGFILE, "a");

    if (file == NULL) {
        if (log_created)
                log_created = 0;
        return;
    }
    else
    {
        time_t now;
        time(&now);
        memset(log_msg, 0, sizeof(log_msg));
        strcpy(log_msg, ctime(&now));
        time_len = strlen(log_msg);
        sprintf(log_msg + time_len - 1, " : %s\n", log_str);
        fputs(log_msg, file);
        fclose(file);
    }
    va_end(ap);
}
