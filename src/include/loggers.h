/** daelib/loggers.h: Sample logging backends.
 */

#ifndef __DAELIB_LOGGERS_H
#define __DAELIB_LOGGERS_H

/* This file defines a handful of
 * logging backends that you can
 * register with dlog.
 * Enjoy.
 */


/* Priority enum. */
#include "log.h"


/* stdout/err loggers. */
/* No init/kill necessary. */
void *stderr_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message);
void *stdout_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message);

/* syslog logger. */
void *syslog_logger_init(void);
void  syslog_logger_kill(void);
void *syslog_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message);


/* logfile logger. */
void *logfile_logger_init(const char *path);
void  logfile_logger_kill(void *ctx);
void *logfile_logger(void *ctx, enum log_priority priority,
                     const char *path, const char *message);



#endif // __DAELIB_LOGGERS_H
