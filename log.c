#include <stdlib.h>
#include <stdio.h>
#include "log.h"

bool log_created = false;

void log (char *message)
{
    FILE *file;
    char log_msg[256];

    if (!log_created) {
        file = fopen(LOGFILE, "w");
        log_created = true;
    }
    else
        file = fopen(LOGFILE, "a");

    if (file == NULL) {
        if (log_created)
                log_created = false;
        return;
    }
    else
    {
        time_t now;
        time(&now);
        memset(log_msg, 0, sizeof(log_msg));
        sprintf(log_msg, "%s : %s\n", ctime(&now), message);
        fputs(log_msg, file);
        fclose(file);
    }

    if (file)
        fclose(file);
}
