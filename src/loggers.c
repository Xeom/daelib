/** daelib/loggers.c: Sample logging backends.
 */


/* Prototypes. */
#include "loggers.h"

/* Logging functions. */
#include "log.h"

/* Asserts. */
#include "assert.h"

/* fprintf(), stderr, fopen(). */
#include <stdio.h>

/* syslog functions. */
#include <syslog.h>

/* Errno. */
#include <errno.h>

/* strerror(). */
#include <string.h>


/* stderr logger. */
void *stderr_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message) {

	/* Attempt to fprintf to stderr.
	 * Format as [Priority] path: message.
	 * Returns and accepts NULL context.
	 */
	fprintf(stderr, "[%s]%s: %s\n", dlog_string(priority), path, message);

	return NULL;
}


/* stdout logger. */
void *stdout_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message) {

	/* Attempt to fprintf to stdout.
	 * Format as [Priority] path: message.
	 * Returns and accepts NULL context.
	 */
	fprintf(stdout, "[%s]%s: %s\n", dlog_string(priority), path, message);

	return NULL;
}


/* syslog logger. */

/* Convert a log_priority
 * to a syslog constant.
 * Returns LOG_INFO on unknown.
 */
static int _syslog_priority(enum log_priority priority) {

	/* Default value,
	 * switch between known priorities,
	 * return.
	 */
	int ret;

	switch(priority) {
	case EEMERG:   ret = LOG_EMERG;   break;
	case EALERT:   ret = LOG_ALERT;   break;
	case ECRIT:    ret = LOG_CRIT;    break;
	case EERR:     ret = LOG_ERR;     break;
	case EWARNING: ret = LOG_WARNING; break;
	case ENOTICE:  ret = LOG_NOTICE;  break;
	case EINFO:    ret = LOG_INFO;    break;
	case EDEBUG:   ret = LOG_DEBUG;   break;
	default:       ret = LOG_INFO;    break;
	}

	return ret;
}

/* Opens the connection to the
 * syslog daemon.
 * TODO: Get program name,
 * ident=NULL is not POSIX defined.
 */
void *syslog_logger_init(void) {

	/* Open syslog for writing.
	 * Silent failure.
	 * Return dummy context.
	 */
	openlog(NULL, LOG_PID, LOG_USER);

	return NULL;
}

/* Close the connection to the
 * syslog daemon.
 * Nothing to see here.
 */
void  syslog_logger_kill(void) {

	/* Closes syslog.
	 * Silent failure.
	 */
	closelog();
}

/* Log a message with syslog.
 * Context is irrelevant. Path
 * and message are joined.
 */
void *syslog_logger(void *ctx, enum log_priority priority,
                    const char *path, const char *message) {

	/* Call syslog with args to
	 * join path / message. Silent
	 * failure.
	 */
	syslog(_syslog_priority(priority),
	       "[%s]: %s\n", path, message);

	return NULL;
}


/* logfile logger. */

/* Open a file for logging.
 * Returns a FILE on success,
 * NULL on error.
 */
void *logfile_logger_init(const char *path) {

	/* Open path for logging.
	 * If success, return FILE*.
	 * On failure, returns NULL.
	 * Log failure with other loggers.
	 */
	FILE *t = fopen(path, "a");

	if (t == NULL)
		dlog(EERR, "dios/logger/file/init",
		     "Error, could not open logfile. Reason: %s.\n",
		     strerror(errno));

	return t;
}

/* Close a logfile.
 * Silent failure.
 */
void  logfile_logger_kill(void *ctx) {

	/* If file is NULL, return.
	 * Else, close, return.
	 */
	DASSERT(ctx != NULL, "Given an invalid context.",
		return;
		);

	fclose(ctx);

	return;
}

/* Log a message in a logfile.
 * Formats as [PRIORITY]path: message.
 * Silent failure.
 */
void *logfile_logger(void *ctx, enum log_priority priority,
                     const char *path, const char *message) {

	/* Check if context is valid.
	 * Prints the entry to the logfile,
	 * then returns. Silent failure.
	 */
	DASSERT(ctx != NULL, "Given an invalid context.",
		return NULL;
		);

	fprintf((FILE*) ctx, "[%s]%s: %s\n", dlog_string(priority),
	        path, message);

	return ctx;
}
