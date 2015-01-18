/* daelib/hashtable.h: Hashtable implementation.
 * TODO: Iterators.
 * TODO: Optional resizing.
 * TODO: Backends.
 */

#ifndef __DAELIB_HASHTABLE_H
#define __DAELIB_HASHTABLE_H

/* Basic hashtable with arbitrary backend.
 * You can find exacting detail in hashtable.c.
 * The backend interface is defined in hashtable_backend.h.
 */


/* size_t. */
#include <stdlib.h>


/* Opaque hashtable structure. */
struct daelib_hashtable;

/* For sanity. */
typedef struct daelib_hashtable *dhtable;


/* Iterator structure. */
struct dhtable_iterator {

	int bucket;
	void *it;
};

/* For sanity. */
typedef struct _dhtable_it dhtable_it;


/* Functors to compare and hash
 * keys. NULL for treating *key as
 * an integral value, and casting
 * to an int. Cmp tests all memory.
 */
typedef int (*dhtable_key_hsh)(size_t key_size, void *key);
typedef int (*dhtable_key_cmp)(size_t key_size, void *keyl, void *keyr);

/* Backend structure. Used in init.
 * You can find example backends in
 * hashtable_backend.h. Ignore this,
 * you can use NULL for sane defaults.
 */
struct dhtable_backend;


/* Hashtable functions. */

/* Init/kill/copy. */
dhtable dhtable_init(size_t buckets, size_t key_size, size_t val_size,
                     dhtable_key_cmp key_cmp, dhtable_key_hsh key_hsh,
                     struct dhtable_backend *backend);
int     dhtable_kill(dhtable table);
dhtable dhtable_copy(dhtable table);

/* Get/Set/Rm. */
void *dhtable_get(dhtable table, void *key);
int   dhtable_put(dhtable table, void *key, void *value);
int   dhtable_rm (dhtable table, void *key);

/* Size/metadata. */
size_t dhtable_size(dhtable table);
size_t dhtable_key_size(dhtable table);
size_t dhtable_val_size(dhtable table);

/* Range operations. */
int dhtable_join(dhtable dst, dhtable src);

/* Iterations. */
dhtable_it dhtable_begin(dhtable table);
dhtable_it dhtable_end  (dhtable table);

dhtable_it dhtable_prev(dhtable table, dhtable_it it);
dhtable_it dhtable_next(dhtable table, dhtable_it it);

void      *dhtable_iget(dhtable table, dhtable_it it);


/* Builtin backends. */
/* TODO: These. */
extern struct dhtable_backend dhtable_vector;
extern struct dhtable_backend dhtable_btree_vector;
extern struct dhtable_backend dhtable_list;
extern struct dhtable_backend dhtable_btree;


#endif // __DAELIB_HASHTABLE_H
