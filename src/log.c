/** daelib/log.c: Simple global logger.
 */


/* Prototypes. */
#include "log.h"

/* Asserts. */
#include "assert.h"

/* Vector. */
#include "vector.h"

/* vargs, vsprintf(). */
#include <stdarg.h>
#include <stdio.h>


/* Logger instance. */
struct _log_logger {

	logger method;
	enum log_priority min_priority;
	void     *ctx;
};


/* Global vector of logger
 * instances.
 */
static dvec _log_loggers = NULL;


/* Default error behaviour. */
#ifndef ICALLER /* When fed bad data. */
#define ICALLER DSLOG
#endif /* ICALLER */

#ifndef IALLOC /* When malloc() fails. */
#define IALLOC DSLOG
#endif /* IALLOC */

#ifndef IVECTOR /* When the vector fails. */
#define IVECTOR DSLOG
#endif /* IVECTOR */


/* Initialize the logging system.
 * Returns nonzero on error.
 * Succeeds on repeat calls.
 */
int dlog_init(void) {

	/* If _log_loggers needs
	 * to be init'd, init it.
	 * If failed, return error.
	 */
	if (_log_loggers == NULL)
		_log_loggers = dvec_init(sizeof(struct _log_logger));

	DASSERT(_log_loggers != NULL, IALLOC, "Failed to create _log_loggers.",
		return 1;
		);

	return 0;
}

/* Add a logger to the global list.
 * Returns nonzero on error.
 */
int dlog_add(logger logger, enum log_priority min_priority, void *ctx) {

	/* Verify initialization,
	 * build a new logger instance,
	 * push to vector, return.
	 */
	DASSERT(_log_loggers != NULL, ICALLER,
		"Log is uninitialized.",
		return 1;
		);

	struct _log_logger new_instance;
	new_instance.method = logger;
	new_instance.min_priority = min_priority;
	new_instance.ctx = ctx;

	int t = dvec_push(_log_loggers, &new_instance);

	DASSERT(t == 0, IVECTOR, "Failed to push new logger.",
		return 1;
		);

	return 0;
}

/* Removes a logger from _log_loggers.
 * Returns nonzero on error. You must
 * know the logger method and context.
 */
int dlog_rm(logger logger, void *ctx) {

	/* Check that _log_loggers is
	 * ready, loop through all
	 * available loggers, if
	 * found, remove.
	 */
	DASSERT(_log_loggers != NULL, ICALLER,
		"Log is uninitialized.",
		return 1;
		);

	size_t count = dvec_size(_log_loggers);

	size_t i;
	for (i = 0; i < count; i++) {

		struct _log_logger *t = (struct _log_logger*)
		                        dvec_get(_log_loggers, i);

		if (t->method == logger && t->ctx == ctx)
			break;
	}

	DASSERT(i != count, ICALLER,
		"Cannot find logger to remove.",
		return 1;
		);

	return dvec_rm(_log_loggers, i);
}

/* Calls each logger instance
 * in _log_loggers. Returns
 * nonzero on error. Format string
 * output limited to 4K bytes.
 */
int dlog(enum log_priority priority, const char *path,
	const char *format, ...) {

	/* Check that _log_loggers is
	 * ready, format the message
	 * using vsnprintf, loop through
	 * the loggers, call when
	 * appropriate, return.
	 */
	DASSERT(_log_loggers != NULL, ICALLER, "Log is uninitialized.",
		return 1;
		);

	va_list args;
	va_start(args, format);

	char buf[4096];
	vsnprintf(buf, sizeof(buf), format, args);

	va_end(args);

	size_t count = dvec_size(_log_loggers);

	size_t i;
	for (i = 0; i < count; i++) {

		struct _log_logger *t = (struct _log_logger*)
		                        dvec_get(_log_loggers, i);

		DASSERT(t != NULL, IVECTOR, "Failed to get logger. Continuing.",
			continue;
			);

		if (priority > t->min_priority)
			break;

		t->ctx = t->method(t->ctx, priority, path, buf);
	}

	return 0;
}

/* Free _log_loggers. */
int dlog_kill(void) {

	/* Check if we need to kill.
	 * If not, kill, check, set
	 * _log_loggers to NULL,
	 * return.
	 */
	if (_log_loggers == NULL)
		return 0;

	int t = dvec_kill(_log_loggers);
	DASSERT(t == 0, IVECTOR, "Failed to kill _log_loggers.",
		return 1;
		);

	_log_loggers = NULL;

	return 0;
}


/* Get a string matching a
 * priority. Returns "?!?!?"
 * on unknown.
 */
const char *dlog_string(enum log_priority priority) {

	/* Start with default,
	 * switch over known priorities,
	 * return.
	 */
	const char *ret;

	switch(priority) {
	case EEMERG:   ret = "EMERG"; break;
	case EALERT:   ret = "ALERT"; break;
	case ECRIT:    ret = "CRIT "; break;
	case EERR:     ret = "ERROR"; break;
	case EWARNING: ret = "WARN "; break;
	case ENOTICE:  ret = "NOTIC"; break;
	case EINFO:    ret = "INFO "; break;
	case EDEBUG:   ret = "DEBUG"; break;
	default:       ret = "?!?!?"; break;
	}

	return ret;
}
