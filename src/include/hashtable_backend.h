/** daelib/hashtable_backend.h: Interface for hashtable backends.
 */

#ifndef __DAELIB_HASHTABLE_BACKEND_H
#define __DAELIB_HASHTABLE_BACKEND_H

/* Hashtables require backends. Here is their interface.
 * Some useful backends are described in hashtable.h.
 */

/* Key functor types */
#include "hashtable.h"


/* Hashtable-specific context
 * for the backends. Keeps each
 * bucket's container from copying
 * the data.
 */
struct dhtable_backend_context {

	size_t key_size;
	size_t val_size;

	dhtable_key_hsh key_hsh;
	dhtable_key_cmp key_cmp;
};

/* For sanity. */
typedef struct dhtable_backend_context dhtable_ctx;


/* Hashtable backend interface. */
typedef void  *(*dhtable_backend_init)(dhtable_ctx *ctx);
typedef int    (*dhtable_backend_kill)(dhtable_ctx *ctx, void *bucket);
typedef void  *(*dhtable_backend_copy)(dhtable_ctx *ctx, void *bucket);

typedef size_t (*dhtable_backend_size)(dhtable_ctx *ctx, void *bucket);

typedef void  *(*dhtable_backend_get)  (dhtable_ctx *ctx, void *bucket,
                                        void *key);
typedef int    (*dhtable_backend_put)  (dhtable_ctx *ctx, void *bucket,
                                        void *key, void *value);
typedef int    (*dhtable_backend_rm)   (dhtable_ctx *ctx, void *bucket,
                                        void *key);

typedef int (*dhtable_backend_join)(dhtable_ctx *ctx, void *dst, void *src);

typedef void *(*dhtable_backend_begin)(dhtable_ctx *ctx, void *bucket);
typedef void *(*dhtable_backend_end)  (dhtable_ctx *ctx, void *bucket);

typedef void *(*dhtable_backend_prev)(dhtable_ctx *ctx, void *bucket, void *it);
typedef void *(*dhtable_backend_next)(dhtable_ctx *ctx, void *bucket, void *it);

typedef void *(*dhtable_backend_iget)(dhtable_ctx *ctx, void *bucket, void *it);

/* Holding structure. */
struct dhtable_backend {

	dhtable_backend_init init;
	dhtable_backend_kill kill;
	dhtable_backend_copy copy;

	dhtable_backend_size size;

	dhtable_backend_get get;
	dhtable_backend_put put;
	dhtable_backend_rm rm;

	dhtable_backend_join join;

	dhtable_backend_begin begin;
	dhtable_backend_end     end;

	dhtable_backend_prev prev;
	dhtable_backend_next next;

	dhtable_backend_iget iget;
};


#endif // __ DAELIB_HASHTABLE_BACKEND_H
