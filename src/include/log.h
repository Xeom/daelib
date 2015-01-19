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

	EEMERG   = 0x01, /* *gurgling blood*           */
	EALERT   = 0x02, /* Halp!!!                    */
	ECRIT    = 0x03, /* Halpppp.                   */
	EERR     = 0x04, /* Error happened.            */
	EWARNING = 0x05, /* Something fishy.           */
	ENOTICE  = 0x06, /* Status report.             */
	EINFO    = 0x07, /* Just-letting-you-know      */
	EDEBUG   = 0x08, /* *rambles on to self*       */

	ENONE    = 0x09  /* Unknown priority.          */
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
