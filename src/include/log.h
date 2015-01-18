/** daelib/log.h: Simple global logger.
 */

#ifndef __DAELIB_LOG_H
#define __DAELIB_LOG_H

/* Simple global logger system.
 * It works with multiple backends.
 * You can find exacting detail in
 * log.c.
 */


/* Log entry structure:
 * A path (module/path/to/system/function/#row:column),
 * severity,
 * message.
 */

/* Message priority. "Inspired" by syslog. */
enum log_priority {

	EEMERG  , /* *gurgling blood*           */
	EALERT  , /* Halp!!!                    */
	ECRIT   , /* Halpppp.                   */
	EERR    , /* Error happened.            */
	EWARNING, /* Something fishy.           */
	ENOTICE , /* Status report.             */
	EINFO   , /* Just-letting-you-know      */
	EDEBUG,   /* *rambles on to self*       */

	ENONE     /* Unknown priority.          */
};


/* Log callback. */
/* Given context, returns new context. */
typedef void *(*logger)(void *ctx, enum log_priority priority,
                        const char *path, const char *message);


/* Init / kill dlog. */
int dlog_init(void);
int dlog_kill(void);

/* Register / remove loggers. */
int dlog_add(logger logger, enum log_priority min_priority, void *ctx);
int dlog_rm(logger logger, void *ctx);

/* Log a message. */
int dlog(enum log_priority priority, const char *path,
         const char *format, ...);

/* Property strings. (5 + \0.). */
const char *dlog_string(enum log_priority priority);


#endif // __DAELIB_LOG_H
