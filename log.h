#define LOGFILE   "transfer.log"     // all Log(); messages will be appended to this file
extern char log_created;      // keeps track whether the log file is created or not
void t_log (const char *format, ...);    // logs a message to LOGFILE

