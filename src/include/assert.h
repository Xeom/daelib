/** daelib/assert.h: Internal dynamic assert system.
 */

#ifndef __DAELIB_ASSERT_H
#define __DAELIB_ASSERT_H

/* assert.h is a bit blunt. These macros
 * allow for chosing assert behaviours for
 * debug, release, and unchecked code.
 * Optionally makes use of dlog().
 */


/* dlog(). */
#include "log.h"

/* Stderr logger. */
#include "loggers.h"

/* exit(). */
#include <stdlib.h>

/* Defines. */

/* You may define one compilation mode.
 * Options are:
 * __DAELIB_DEBUG: Maximum output, total checking.
 * __DAELIB_RELEASE: Medium output, medium checking.
 * __DAELIB_STRIP: Minimal output, little checking.
 * Debug is the default mode. It logs everything,
 * checks everything, and allows asserts to exit()
 * on failure. It is used when code is being tested.
 * Release is akin to debug, but you may supress logging
 * and exit()s. Strip removes all asserts, except for
 * external input using DASSERT_NS (NoStrip.)
 */

/* Constants representing various
 * build modes.
 */
#define DDEBUG   0x01
#define DRELEASE 0x02
#define DSTRIP   0x04

#define DNEVER   0x00
#define DALWAYS  0x07


/* Define __DAELIB_MODE as a
 * number with the current
 * release version.
 */
#undef __DAELIB_MODE

#if   defined (__DAELIB_STRIP)
#define __DAELIB_MODE DSTRIP

#elif defined (__DAELIB_RELEASE)
#define __DAELIB_MODE DRELEASE

#elif defined (__DAELIB_DEBUG)
#define __DAELIB_MODE DDEBUG

#else
#define __DAELIB_MODE DDEBUG

#endif


/* Basic assertions. */

/* Assert, using dlog(). */
#define DASSERT_NS_F(cond, logmode, exitmode, priority, msg, action) \
    do {                                                             \
        if (!(cond)) {                                               \
            if (__DAELIB_MODE & DDEBUG || (logmode & __DAELIB_MODE))     \
                dlog(priority, __FILE__, "L%d: " msg, __LINE__);     \
            if (exitmode & __DAELIB_MODE)                              \
                exit(EXIT_FAILURE);                                  \
            action                                                   \
        }                                                            \
    } while (0)

/* Assert, using printf(). */
#define DASSERT_NS_NL_F(cond, logmode, exitmode, priority, msg, action) \
    do {                                                                \
        if (!(cond)) {                                                  \
            if (__DAELIB_MODE & DDEBUG || (logmode & __DAELIB_MODE))        \
                stdout_logger(NULL, priority, __FILE__, msg);           \
            if (exitmode & __DAELIB_MODE)                                 \
                exit(EXIT_FAILURE);                                     \
            action                                                      \
        }                                                               \
    } while (0)


/* Derived assertion functions. Use these. */

/* Asserts that can be stripped. */
#ifdef __DAELIB_STRIP
#define DASSERT_F(cond, logmode, exitmode, priority, msg, action)
#define DASSERT_NL_F(cond, logmode, exitmode, priority, msg, action)

#else // __DAELIB_STRIP
#define DASSERT_F(cond, logmode, exitmode, priority, msg, action) \
    DASSERT_NS_F(cond, logmode, exitmode, priority, msg, action)
#define DASSERT_NL_F(cond, logmode, exitmode, priority, msg, action) \
    DASSERT_NS_NL_F(cond, logmode, exitmode, priority, msg, action)

#endif // !__DAELIB_STRIP


#define DASSERT_D(cond, msg, action) \
    DASSERT_F(cond, DDEBUG, DNEVER, EWARNING, msg, action)
#define DASSERT_P(cond, msg, action) \
    DASSERT_F(cond, DALWAYS, DNEVER, EWARNING, msg, action)
#define DASSERT(cond, msg, action) \
    DASSERT_D(cond, msg, action)

#define DASSERT_NL_D(cond, msg, action) \
    DASSERT_NL_F(cond, DDEBUG, DNEVER, EERR, msg, action)
#define DASSERT_NL_P(cond, msg, action) \
    DASSERT_NL_F(cond, DALWAYS, DNEVER, EERR, msg, action)
#define DASSERT_NL(cond, msg, action) \
    DASSERT_NL_D(cond, msg, action)

#define DASSERT_NS_D(cond,msg, action) \
    DASSERT_NS_F(cond, DDEBUG, DNEVER, EERR, msg, action)
#define DASSERT_NS_P(cond, msg, action) \
    DASSERT_NS_F(cond, DALWAYS, DNEVER, EERR, msg, action)
#define DASSERT_NS(cond, msg, action) \
    DASSERT_NS_DD(cond, msg, action)

#define DASSERT_NS_NL_D(cond, msg, action) \
    DASSERT_NS_NL_F(cond, DDEBUG, DNEVER, EERR, msg, action)
#define DASSERT_NS_NL_P(cond, msg, action) \
    DASSERT_NS_NL_F(cond, DALWAYS, DNEVER, EERR, msg, action)
#define DASSERT_NS_NL(cond, msg, action) \
    DASSERT_NS_NL_D(cond, msg, action)


#endif // __DAELIB_ASSERT_H
