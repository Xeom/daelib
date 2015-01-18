/** daelib/test.c: Test programme.
 */

/* printf(). */
#include <stdio.h>

/* exit(). */
#include <stdlib.h>

/* strlen(). */
#include <string.h>

/* Vectors. */
#include "vector.h"

/* Hastable. */
#include "hashtable.h"

/* Logging. */
#include "log.h"
#include "loggers.h"

/* Assertions. */
#include "assert.h"


void cleanup(void);

void init_loggers(void);
void kill_loggers(void);

void test_hashtable(void);
void test_vector(void);

void test_assert(void);

int main() {

	init_loggers();

	dlog(EINFO, "test/init", "Startup complete.");

	test_hashtable();

	test_vector();

	test_assert();

	dlog(EINFO, "test/term", "Successfully completed tests. Exiting.");

	cleanup();
	exit(EXIT_SUCCESS);
}

void cleanup(void) {

	kill_loggers();
}

void *logfile_ctx = NULL;

void init_loggers(void) {

	if (dlog_init() != 0) {
		printf("Failed to initialize dlog.\n");
		return;
	}

	if (dlog_add(&stdout_logger, EDEBUG, NULL) != 0)
		printf("Failed to add stderr logger to dlog.\n");

	if (dlog_add(&stderr_logger, EWARNING, NULL) != 0)
		printf("Failed to add stderr logger to dlog.\n");

	syslog_logger_init();
	if (dlog_add(&syslog_logger, ENOTICE, NULL) != 0)
		printf("Failed to add syslog logger to dlog.\n");

	logfile_ctx = logfile_logger_init("/tmp/dios.log");
	if (dlog_add(&logfile_logger, EINFO, logfile_ctx) != 0)
		printf("Failed to add logfile backend.\n");

	return;
}

void kill_loggers() {

	syslog_logger_kill();
	logfile_logger_kill(logfile_ctx);

	dlog_kill();
}

void test_hashtable(void) {

	dlog(EINFO, "test/hashtable", "Starting hashtable tests.");
	dhtable table = dhtable_init((1 << 1), sizeof(int), sizeof(int), NULL, NULL, NULL);
	DASSERT(table != NULL, "Failed to init table.",
		return;
		);

	int i, j;

	j = 1;
	dlog(EINFO, "test/hashtable", "Starting insertion.");
	for (i = 0; i < (1 << 2); i++)
		if (dhtable_put(table, &i, &j) != 0)
			dlog(EERR, "test/hashtable", "Failed to put element.");
	dlog(EINFO, "test/hashtable", "Starting searching.");
	for (i = 0; i < (1 << 2); i++)
		if (dhtable_get(table, &i) == NULL)
			dlog(EERR, "test/hashtable", "Failed to get element.");

	j = 2;
	void *t;
	if ((t = dhtable_get(table, &j)) == NULL)
		dlog(EERR, "test/hashtable", "Failed to get element.");

	dlog(EINFO, "test/hashtable", "Maps from 2 -> %d.", *((int*) t));

	j = 1;
	dlog(EINFO, "test/hashtable", "Starting table clone.");
	dhtable table2 = dhtable_copy(table);
	if (table2 == NULL)
		dlog(EERR, "test/hashtable", "Failed to kill table.");
	dlog(EINFO, "test/hashtable", "Elements: New: %d, old: %d.",
	     dhtable_size(table2), dhtable_size(table));


	dlog(EINFO, "test/hashtable", "Inserting misc elements into src...");
	i = 1 << 3;
	if (dhtable_put(table, &i, &j) != 0)
		dlog(EERR, "test/hashtable", "Failed to put element.");
	i = 1 << 4;
	if (dhtable_put(table, &i, &j) != 0)
		dlog(EERR, "test/hashtable", "Failed to put element.");


	dlog(EINFO, "test/hashtable", "Starting hashtable join.");
	if (dhtable_join(table2, table) != 0)
		dlog(EERR, "test/hashtable", "Failed to join tables.");
	dlog(EINFO, "test/hashtable", "Elements: New: %d, old: %d.",
	     dhtable_size(table2), dhtable_size(table));

	dlog(EINFO, "test/hashtable", "Removing random elements.");
	i = 1;
	if (dhtable_rm(table2, &i) != 0)
		dlog(EERR, "test/hashtable", "Failed to remove element.");
	i = 2;
	if (dhtable_rm(table2, &i) != 0)
		dlog(EERR, "test/hashtable", "Failed to remove element.");
	dlog(EINFO, "test/hashtable", "Elements: New: %d, old: %d.",
	     dhtable_size(table2), dhtable_size(table));

	if (dhtable_get(table2, &i) != NULL)
		dlog(EERR, "test/hashtable", "Got rm'd element.");

	dlog(EINFO, "test/hashtable", "Key, valsize: %d, %d.",
	     dhtable_key_size(table), dhtable_val_size(table));

	dlog(EINFO, "test/hashtable", "Starting hashtable kill.");
	if (dhtable_kill(table) != 0)
		dlog(EERR, "test/hashtable", "Failed to kill table.");
	if (dhtable_kill(table2) != 0)
		dlog(EERR, "test/hashtable", "Failed to kill table.");

	dlog(EINFO, "test/hashtable", "Finished tests.");
}

void test_vector(void) {


}

void test_assert(void) {

	dlog(EWARNING, "test/assert", "Testing dassert failures.");
	dvec t = dvec_copy(NULL);
	t = dvec_init(sizeof(char));
	dvec_peek(t);
	dvec_push(t, "a");
	dvec_push(t, "b");
	dvec_get(t, 20);
	dvec_pop(t);
	dvec_pop(t);
	dvec_pop(t);
	dlog(EWARNING, "test/dassert", "Finished testing.");
}
