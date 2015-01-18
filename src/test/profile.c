/** dios/profile.c: Test programme.
 */

/* printf(). */
#include <stdio.h>

/* exit(). */
#include <stdlib.h>

/* clock_gettime(). */
#include <time.h>

/* vectors. */
#include "vector.h"

/* Hashtable. */
#include "hashtable.h"

/* logging. */
#include "log.h"
#include "loggers.h"


#define CLOCK CLOCK_PROCESS_CPUTIME_ID

void profile_init(void);
void profile_kill(void);

void profile_vector(void);
void profile_hashtable(void);

int main() {

	profile_init();

	profile_vector();
	profile_hashtable();

	profile_kill();

	return 0;
}


void profile_init(void) {

	dlog_init();
	dlog_add(&stdout_logger, EDEBUG, NULL);

	struct timespec t;
	clock_getres(CLOCK, &t);
	dlog(EINFO, "profile/init", "Clock resolution: %d ns.", t.tv_nsec);

	dlog(EINFO, "profile/init", "Init completed.");
}

void profile_kill(void) {

	dlog(EINFO, "profile/term", "Tests completed. Exiting.");

	dlog_kill();

	exit(EXIT_SUCCESS);
}


void profile_vector(void) {

	struct timespec start, end;

	dlog(EINFO, "profile/vector/stack/t1", "push() x 1mil, pop() x 1mil.");

	clock_gettime(CLOCK, &start);

	dvec v1 = dvec_init(sizeof(char));

	int i;
	for (i = 0; i < (1 << 20); i++)
		dvec_push(v1, "a");

	for (i = 0; i < (1 << 20); i++)
		dvec_pop(v1);

	dvec_kill(v1);

	clock_gettime(CLOCK, &end);


	dlog(EINFO, "profile/vector/stack/t1", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);

	
	dlog(EINFO, "profile/vector/stack/t2", "push() peek() pop() x 1mil.");

	clock_gettime(CLOCK, &start);

	dvec v2 = dvec_init(sizeof(char));

	for (i = 0; i < (1 << 20); i++) {
		dvec_push(v2, "a");
		dvec_peek(v2);
		dvec_pop(v2);
	}

	dvec_kill(v2);

	clock_gettime(CLOCK, &end);

	dlog(EINFO, "profile/vector/stack/t2", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);


	dlog(EINFO, "profile/vector/stack/t3", "push() x 1mil, get() x 1mil.");

	clock_gettime(CLOCK, &start);

	dvec v3 = dvec_init(sizeof(char));

	for (i = 0; i < (1 << 20); i++)
		dvec_push(v3, "a");

	for (i = 0; i < (1 << 20); i++)
		dvec_get(v3, i);

	/* dvec_kill(v3); */

	clock_gettime(CLOCK, &end);

	dlog(EINFO, "profile/vector/stack/t3", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);

	
	dlog(EINFO, "profile/vector/stack/t4", "for 10000:pushx1000,popx1000.");

	clock_gettime(CLOCK, &start);

	dvec v4 = dvec_init(sizeof(char));

	for (i = 0; i < 10000; i++) {
		int i2;
		for (i2 = 0; i2 < 1000; i2++)
			dvec_push(v4, "a");
		for (i2 = 0; i2 < 1000; i2++)
			dvec_pop(v4);
	}

	clock_gettime(CLOCK, &end);

	dlog(EINFO, "profile/vector/stack/t4", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);


	dlog(EINFO, "profile/vector/range/t1", "Joining two 1m vecs.");

	clock_gettime(CLOCK, &start);

	dvec v5 = dvec_copy(v3);

	dvec_join(v5, v3, (1 << 20));

	dvec_kill(v3);
	dvec_kill(v5);

	clock_gettime(CLOCK, &end);

	dlog(EINFO, "profile/vector/range/t1", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);

	return;
}

void profile_hashtable(void) {

	struct timespec start, end;

	dhtable table = dhtable_init((1 << 6), sizeof(int), 0, NULL, NULL, NULL);

	dlog(EINFO, "profile/hashtable/t1", "put() x 16k, get() x 16k.");

	clock_gettime(CLOCK, &start);

	dlog(EINFO, "profile/hashtable/t1", "Starting inserting x 16k.");
	int i;
	for (i = 0; i < (1 << 14); i++)
		if (dhtable_put(table, &i, NULL) != 0)
			dlog(EERR, "profile/hashtable/t1", "Failed to put element.");

	dlog(EINFO, "profile/hashtable/t1", "Starting searching x 16k.");
	for (i = 0; i < (1 << 14); i++)
		if (dhtable_get(table, &i) == NULL)
			dlog(EERR, "profile/hashtable/t1", "Failed to get element.");

	if (dhtable_kill(table) != 0)
		dlog(EERR, "profile/hashtable/t1", "Failed to kill table.");

	clock_gettime(CLOCK, &end);

	dlog(EINFO, "profile/hashtable/t1", "Done. Time: %d ns.",
	     end.tv_nsec - start.tv_nsec);
}

