/** daelib/assert.h: Internal dynamic assert system.
 */

#ifndef __DAELIB_ASSERT_H
#define __DAELIB_ASSERT_H

/* The standard assert.h is a bit blunt.
 * This system allows you to do both assertion
 * and error checking while configuring the
 * behaviour on a per-interface basis.
 * You may log, printf, exit, and enter error
 * handling code.
 */

/* dlog(). */
#include "log.h"

/* Stderr logger. */
#include "loggers.h"

/* exit(). */
#include <stdlib.h>


/* Assertions take an assertion, an error string,
 * a flag int, and error handling code.
 * It is recommended you have one flag constant per
 * interface, and use a -Define argument to change
 * the behaviour. There are some presets below.
 */


/* Available actions: */

/* Strip the check. */
#define DSTRIP 0x10

/* Log using the dlog system. */
#define DLOG 0x20

/* Log using stdout. */
#define DSLOG 0x40

/* Exit on trigger. */
#define DEXIT 0x80


/* Assert / check. */
#define DASSERT(cond, flags, msg, action)                        \
  do {                                                           \
    if (!(flags & DSTRIP))                                       \
      if (!(cond)) {                                             \
	int __log_priority = flags & 0x0F;                       \
	if (__log_priority == 0)                                 \
	  __log_priority = EWARNING;                             \
	if (flags & DLOG)                                        \
	  dlog(__log_priority, __FILE__, "L%d: " msg, __LINE__); \
	if (flags & DSLOG)                                       \
	  stdout_logger(NULL, __log_priority, __FILE__, msg);    \
	if (flags & DEXIT)                                       \
	  exit(EXIT_FAILURE);                                    \
	action                                                   \
      }                                                          \
  } while (0)

#endif // __DAELIB_ASSERT_H
