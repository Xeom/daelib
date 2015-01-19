/** daelib/hashtable.c: Hashtable implementation.
 * TODO: Consider invalidating data before free().
 * TODO: Free containers when empty.
 */


/* Prototypes. */
#include "hashtable.h"

/* Backend interface. */
#include "hashtable_backend.h"

/* Assertions. */
#include "assert.h"

/* malloc(), realloc(), free(). */
#include <stdlib.h>

/* memcpy(), memset(). */
#include <string.h>


/* Base definition of a hashtable. */
struct daelib_hashtable {

	struct dhtable_backend *backend;
	struct dhtable_backend_context kv_data;
	size_t bucket_count;
	void **buckets;
};


/* Default error behaviour. */
#ifndef ICALLER /* When fed bad data. */
#define ICALLER DLOG
#endif /* ICALLER */

#ifndef IINTRA /* When table is invalid. */
#define IINTRA DSTRIP
#endif /* IINTRA */

#ifndef IALLOC /* When malloc() fails. */
#define IALLOC DLOG
#endif /* IALLOC */

#ifndef IBACKEND /* When the backend fails. */
#define IBACKEND DLOG
#endif /* IBACKEND */


/* Determine if a hashtable is valid.
 * If valid return nonzero. Else return zero.
 */
static int _dhtable_valid(dhtable table) {

	/* Check for valid keysize, functors,
	 * and buckets.
	 */
	if (table->backend == NULL)
		return 0;

	if (table->bucket_count == 0)
		return 0;

	if (table->buckets == NULL)
		return 0;

	if (table->kv_data.key_size == 0)
		return 0;

	if (table->kv_data.key_cmp == NULL)
		return 0;

	if (table->kv_data.key_hsh == NULL)
		return 0;

	return 1;
}

/* Checks if two tables have the same types.
 * Returns nonzero on joinable, else zero.
 */
static int _dhtable_joinable(dhtable dst, dhtable src) {

	/* Validate each dhtable, check for
	 * the same kv_data, backend,
	 * bucket_count.
	 */
	if (!_dhtable_valid(dst) || !_dhtable_valid(src))
		return 1;

	if (dst->bucket_count != src->bucket_count)
		return 0;

	if (dst->backend != src->backend)
		return 0;

	if (dst->kv_data.key_size != src->kv_data.key_size)
		return 0;

	if (dst->kv_data.val_size != src->kv_data.val_size)
		return 0;

	if (dst->kv_data.key_cmp != src->kv_data.key_cmp)
		return 0;

	if (dst->kv_data.key_hsh != src->kv_data.key_hsh)
		return 0;

	return 1;
}

/* Basic memory compare function. */
int _dhtable_key_cmp(size_t key_size, void *keyl, void *keyr) {

	/* Validate the keys,
	 * compare the block of memory,
	 * return.
	 */
	DASSERT(keyl != NULL, ICALLER, "Given invalid left key.",
		return -1;
		);

	DASSERT(keyr != NULL, ICALLER, "Given invalid right key.",
		return -1;
		);

	return memcmp(keyl, keyr, key_size);
}

/* Treat the key as an int, return that. */
int _dhtable_key_hsh(size_t key_size, void *key) {

	/* Validate the key,
	 * return the first part of the
	 * key as an int.
	 */
	DASSERT(key != NULL, ICALLER, "Given invalid key.",
		return 0;
		);

	return *((int*)key);
}

/* Allocate and initialize a hashtable.
 * TODO: If size == 0, dynamically allocate.
 */
dhtable dhtable_init(size_t buckets, size_t key_size, size_t val_size,
                     dhtable_key_cmp key_cmp, dhtable_key_hsh key_hsh,
                     struct dhtable_backend *backend) {

	/* Validate arguments, allocate memory, allocate
	 * buckets, clean buckets, deal with defaults,
	 * set parameters, return.
	 */
	DASSERT(key_size != 0, ICALLER, "Given bad key_size.",
		return NULL;
		);

	DASSERT(buckets != 0, ICALLER, "Given bad bucket count.",
		return NULL;
		);

	dhtable new_table = (dhtable) malloc(sizeof(struct daelib_hashtable));
	DASSERT(new_table != NULL, IALLOC, "Failed to allocate new table.",
		return NULL;
		);

	void **new_buckets = (void**) malloc(sizeof(void*) * buckets);
	DASSERT(new_buckets != NULL, IALLOC, "Failed to allocate new buckets.",
		free(new_table);
		return NULL;
		);

	memset(new_buckets, 0, sizeof(void*) * buckets);

	if (key_cmp == NULL)
		key_cmp = &_dhtable_key_cmp;
	if (key_hsh == NULL)
		key_hsh = &_dhtable_key_hsh;
	if (backend == NULL)
		backend = &dhtable_vector;

	new_table->backend = backend;
	new_table->buckets = new_buckets;
	new_table->bucket_count = buckets;
	new_table->kv_data.key_size = key_size;
	new_table->kv_data.val_size = val_size;
	new_table->kv_data.key_hsh = key_hsh;
	new_table->kv_data.key_cmp = key_cmp;

	return new_table;
}

/* For each valid bucket, call backend->kill,
 * free the buckets, invalidate the struct,
 * free the struct.
 */
int dhtable_kill(dhtable table) {

	/* Validate the dhtable,
	 * for each bucket, call kill,
	 * ignore failures, invalidate
	 * members, free buckets, free table.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 1;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 1;
		);

	int count = table->bucket_count;

	int i;
	for (i = 0; i < count; i++) {

		if (table->buckets[i] != NULL) {

			int t = table->backend->
			        kill(&table->kv_data, table->buckets[i]);

			DASSERT(t == 0, IBACKEND, "Failed to kill bucket.",
				return 1;
				);
		}
	}

	free(table->buckets);

	table->buckets = NULL;

	free(table);

	return 0;
}

/* Free all buckets before index,
 * free buckets, free table.
 * ASSUMES VALID TABLE, INDEX.
 */
static void _dhtable_kill_copy(dhtable table, void **buckets, int index) {

	/* For each entry,
	 * if valid, kill, free buckets,
	 * invalidate table, free table.
	 */
	int i;
	for (i = index; i >= 0; i--) {

		if (buckets[i] != NULL) {
			int t = table->backend->kill(&table->kv_data, buckets[i]);

			DASSERT(t == 0, IBACKEND, "Failed to kill bucket. Continuing.",
				continue;
				);
		}
	}

	free(buckets);
	free(table);
}

/* Copy a hashtable.
 * Returns NULL on error.
 */
dhtable dhtable_copy(dhtable table) {

	/* Validate the hashtable,
	 * allocate the new table, buckets,
	 * for each bucket, call backend->copy,
	 * if fail, kill all previous buckets,
	 * copy metadata, return.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return NULL;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return NULL;
		);

	dhtable new_table = (dhtable) malloc(sizeof(struct daelib_hashtable));
	DASSERT(new_table != NULL, IALLOC, "Failed to allocate new table.",
		return NULL;
		);

	void **new_buckets = malloc(sizeof(void*) * table->bucket_count);
	DASSERT(new_buckets != NULL, IALLOC, "Failed to allocate new buckets.",
		free(new_buckets);
		return NULL;
		);

	int count = table->bucket_count;

	int i;
	for (i = 0; i < count; i++) {

		void *current_bucket = table->buckets[i];
		void *new_bucket = NULL;

		if (current_bucket == NULL) {
			new_buckets[i] = new_bucket;
			continue;
		}

		new_bucket = table->backend->copy(&table->kv_data,
		                               current_bucket);

		DASSERT(new_bucket != NULL, IBACKEND, "Failed to copy a bucket.",
			_dhtable_kill_copy(table, new_buckets, i - 1);
			return NULL;
			);

		new_buckets[i] = new_bucket;
	}

	memcpy(new_table, table, sizeof(struct daelib_hashtable));
	new_table->buckets = new_buckets;

	return new_table;
}

/* Get the hash of the key,
 * find the bucket, call the backend,
 * return the result.
 */
void *dhtable_get(dhtable table, void *key) {

	/* Validate the table,
	 * get the hash, get the bucket,
	 * check the bucket, call the
	 * backend, return.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return NULL;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return NULL;
		);

	DASSERT(key != NULL, ICALLER, "Given invalid key.",
		return NULL;
		);

	int hash = table->kv_data.key_hsh(table->kv_data.key_size, key);

	int index = ((unsigned) hash) % table->bucket_count;

	void *bucket = table->buckets[index];

	if (bucket == NULL)
		return NULL;

	return table->backend->get(&table->kv_data, bucket, key);
}

/* Hash the key, index
 * into the table, call the
 * backend, return the status.
 */
int dhtable_put(dhtable table, void *key, void *value) {

	/* Validate the table, key,
	 * hash the key, mod the count,
	 * get the bucket, call backend->put,
	 * return.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 1;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 1;
		);

	DASSERT(key != NULL, ICALLER, "Given invalid key.",
		return 1;
		);

	int hash = table->kv_data.key_hsh(table->kv_data.key_size, key);

	int index = ((unsigned) hash) % table->bucket_count;

	void *bucket = table->buckets[index];

	if (bucket == NULL) {
		bucket = table->backend->init(&table->kv_data);

		DASSERT(bucket != NULL, IBACKEND, "Failed to create a bucket.",
			return 1;
			);

		table->buckets[index] = bucket;
	}

	return table->backend->put(&table->kv_data, bucket, key, value);
}

/* Hash the key, index
 * into the table, call the
 * backend, return the status.
 */
int dhtable_rm (dhtable table, void *key) {

	/* Validate the table, key
	 * hash the key, mod the bucket_count,
	 * index into the table, verify
	 * the bucket, call the backend,
	 * return.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 1;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 1;
		);

	DASSERT(key != NULL, ICALLER, "Given invalid key.",
		return 1;
		);

	int hash = table->kv_data.key_hsh(table->kv_data.key_size, key);

	int index = ((unsigned) hash) % table->bucket_count;

	void *bucket = table->buckets[index];

	if (bucket == NULL)
		return 0;

	return table->backend->rm(&table->kv_data, bucket, key);
}

/* Returns the element count. */
size_t dhtable_size(dhtable table) {

	/* Validate the table, for each bucket,
	 * get the number of elements,
	 * return sum.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 0;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 0;
		);

	size_t n = 0;

	size_t count = table->bucket_count;

	size_t i;
	for (i = 0; i < count; i++) {

		void *bucket = table->buckets[i];

		if (bucket != NULL)
			n += table->backend->size(&table->kv_data, bucket);
	}

	return n;
}

/* Returns the key size. */
size_t dhtable_key_size(dhtable table) {

	/* Validate the table,
	 * return the key size.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 0;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 0;
		);

	return table->kv_data.key_size;
}

/* Returns the value size */
size_t dhtable_val_size(dhtable table) {

	/* Validate the table,
	 * return the value size.
	 */
	DASSERT(table != NULL, ICALLER, "Given NULL table.",
		return 0;
		);

	DASSERT(_dhtable_valid(table), IINTRA, "Given invalid table.",
		return 0;
		);

	return table->kv_data.val_size;
}

/* For each corresponding
 * bucket pair in two hashtables,
 * join one into the other.
 */
int dhtable_join(dhtable dst, dhtable src) {

	/* Validate that buckets are
	 * joinable, for each bucket,
	 * attempt backend->join, return.
	 */
	DASSERT(dst != NULL, ICALLER, "Given NULL destination table.",
		return 1;
		);

	DASSERT(src != NULL, ICALLER, "Given NULL source table.",
		return 1;
		);

	DASSERT(_dhtable_joinable(dst, src), ICALLER, "Given unjoinable tables.",
		return 1;
		);

	void **dbuckets = dst->buckets;
	void **sbuckets = src->buckets;

	int count = dst->bucket_count;

	int i;
	for (i = 0; i < count; i++) {

		void *sbucket = sbuckets[i];

		if (sbucket == NULL)
			continue;

		void *dbucket = dbuckets[i];

		if (dbucket == NULL) {
			dbucket = dst->backend->init(&dst->kv_data);
			dbuckets[i] = dbucket;
		}

		DASSERT(dbucket != NULL, IBACKEND, "Failed to create a bucket.",
			return 1;
			);

		int t = dst->backend->join(&dst->kv_data, dbucket, sbucket);

		DASSERT(t == 0, IBACKEND, "Failed to join two buckets.",
			return 1;
			);
	}

	return 0;
}

dhtable_it dhtable_begin(dhtable table);
dhtable_it dhtable_end  (dhtable table);

dhtable_it dhtable_prev(dhtable table, dhtable_it it);
dhtable_it dhtable_next(dhtable table, dhtable_it it);

void      *dhtable_iget(dhtable table, dhtable_it it);
