#include <stdlib.h>
#include <stdio.h>
#include "log.h"

char log_created = 0;

void t_log (char *message)
{
    FILE *file;
    char log_msg[256];

    if (!log_created) {
        file = fopen(LOGFILE, "w");
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
        sprintf(log_msg, "%s\b : %s\n", ctime(&now), message);
        fputs(log_msg, file);
        fclose(file);
    }

    if (file)
        fclose(file);
}
