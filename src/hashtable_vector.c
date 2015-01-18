/** daelib/hashtable_vector.c: Vector backend for hashtable.
 * TODO: Decide on iterators for vectors, or
 * allow range treatment. Get is *really* inefficient
 * until then.
 */


/* We don't need to store any extra data for
 * vectors, so the bucket is just a dvec.
 * Since we're given a dhtable_ctx, we have
 * the correct sizes.
 */


/* Prototypes. */
#include "hashtable_backend.h"

/* Vectors. */
#include "vector.h"

/* Assertions. */
#include "assert.h"

/* malloc(), realloc(), free(). */
#include <stdlib.h>

/* memcpy(). */
#include <string.h>


/* Backend functions. */
void *dhtable_vector_init(dhtable_ctx *ctx);
int   dhtable_vector_kill(dhtable_ctx *ctx, void *bucket);
void *dhtable_vector_copy(dhtable_ctx *ctx, void *bucket);

size_t dhtable_vector_size(dhtable_ctx *ctx, void *bucket);

void *dhtable_vector_get(dhtable_ctx *ctx, void *bucket,
                         void *key);
int   dhtable_vector_put(dhtable_ctx *ctx, void *bucket,
                         void *key, void *value);
int   dhtable_vector_rm (dhtable_ctx *ctx, void *bucket,
                         void *key);

int dhtable_vector_join(dhtable_ctx *ctx, void *dst, void *src);


/* Hashtable backend struct. */
struct dhtable_backend dhtable_vector = {
	.init = dhtable_vector_init,
	.kill = dhtable_vector_kill,
	.copy = dhtable_vector_copy,

	.size = dhtable_vector_size,

	.get = dhtable_vector_get,
	.put = dhtable_vector_put,
	.rm = dhtable_vector_rm,

	.join = dhtable_vector_join
};


/* A note on assertions:
 * Hashtable and vector each have
 * their own assertions on their
 * respective interfaces. You should
 * be asserting the details that
 * will *not* be represented in the other
 * party's interface. For example, validating
 * dhtable_ctx's.
 * You *should* pass a bad bucket.
 * If your logic will depend on it, do validate.
 */

/* Validate a context. */
static int _dhtable_ctx_valid(dhtable_ctx *ctx) {

	/* Validate painter,
	 * all fields but val_size
	 * (you can have empty
	 * value), return.
	 */
	if (ctx == NULL)
		return 0;
	if (ctx->key_size == 0)
		return 0;
	if (ctx->key_cmp == NULL)
		return 0;
	if (ctx->key_hsh == NULL)
		return 0;

	return 1;
}

/* Search for a key.
 * If not found, return -1.
 * TODO: Decide on continuity or iterators
 * for vectors.
 */
static int _dhtable_vector_search(dhtable_ctx *ctx, dvec vec, void *key) {

	/* TODO: See if I ensure continuity in vectors.
	 * Until then, use the direct interface.
	 * This code fragment uses continuity, replace
	 * the next segment when I decide on iterators or
	 * continuity.
	 */
	size_t count = dvec_size(vec);

	/* Start version two.

	if (count == 0)
		return -1;

	size_t elem_size = dvec_elem_size(vec);

	DASSERT(elem_size != 0, "Received invalid elem_size.",
		return -1;
		);

	char *base = (char*) dvec_get(vec, 0);

	DASSERT(t != NULL, "Failed to get first element.",
		return NULL;
		);

	size_t i;
	for (i = 0; i < count; i++)

		char *t = base + i * elem_size;

		if (ctx->key_cmp(ctx->key_size, key, (void*) t) == 0)
			return i;
	}
	* End version two.
	* Start version one.
	*/

	size_t i;
	for (i = 0; i < count; i++) {

		char *t = dvec_get(vec, i);

		DASSERT(t != NULL, "Failed to get element.",
			return -1;
			);

		if (ctx->key_cmp(ctx->key_size, key, (void*) t) == 0)
			return (int) i;
	}
	/* End version one. */

	return -1;
}

/* Replace the value of
 * a map element.
 */
static int _dhtable_vector_replace(dhtable_ctx *ctx, dvec vec,
                                   void *value, int t) {

	char *elem = (char*) dvec_get(vec, t);

	DASSERT(elem != NULL, "Failed to get element.",
		return 1;
		);

	char *val = elem + ctx->key_size;

	memcpy(val, value, ctx->val_size);

	return 0;
}

/* Insert a key, value pair
 * at end.
 */
static int _dhtable_vector_push(dhtable_ctx *ctx, dvec vec,
                                void *key, void *value) {

	/* Create buffer, memcpy key,
	 * if value, memcpy, push,
	 * return.
	 */
	size_t size = dvec_elem_size(vec);

	DASSERT(size != 0, "Received invalid elem_size.",
		return 1;
		);

	char buff[size];

	memcpy(buff, key, ctx->key_size);

	if (ctx->val_size != 0)
		memcpy(buff + ctx->key_size, value, ctx->val_size);

	return dvec_push(vec, (void*) buff);
}

/* Initialize a bucket. */
void *dhtable_vector_init(dhtable_ctx *ctx) {

	/* Validate ctx, init vector,
	 * return.
	 */
	DASSERT(_dhtable_ctx_valid(ctx) != 0, "Given invalid context.",
		return NULL;
		);

	return (void*) dvec_init(ctx->key_size + ctx->val_size);
}

/* Free a bucket. */
int dhtable_vector_kill(dhtable_ctx *ctx, void *bucket) {

	/* Call dvec_kill,
	 * return error.
	 */
	return dvec_kill((dvec) bucket);
}

/* Copy a bucket. */
void *dhtable_vector_copy(dhtable_ctx *ctx, void *bucket) {

	/* Call dvec_copy,
	 * return bucket.
	 */
	return (void*) dvec_copy((dvec) bucket);
}

/* Get the element count of a bucket. */
size_t dhtable_vector_size(dhtable_ctx *ctx, void *bucket) {

	/* Call dvec_size,
	 * return.
	 */
	return dvec_size((dvec) bucket);
}

/* Get an element in a bucket. */
void *dhtable_vector_get(dhtable_ctx *ctx, void *bucket, void *key) {

	/* Verify the context, verify the key,
	 * search for element, get element,
	 * verify, get val, return.
	 */
	DASSERT(_dhtable_ctx_valid(ctx), "Given invalid context.",
		return NULL;
		);

	DASSERT(key != NULL, "Given invalid key.",
		return NULL;
		);

	dvec vec = (dvec) bucket;

	int t = _dhtable_vector_search(ctx, vec, key);

	if (t < 0)
		return NULL;

	char *r = (char*) dvec_get(vec, t);

	DASSERT(r != NULL, "Failed to get element.",
		return NULL;
		);

	void *value = r + ctx->key_size;

	return value;
}

/* Push an element onto a bucket.
 */
int dhtable_vector_put(dhtable_ctx *ctx, void *bucket,
                       void *key, void *value) {

	/* Verify context, key,
	 * if exists, overwrite
	 * value or push new key, value.
	 * return.
	 */
	DASSERT(_dhtable_ctx_valid(ctx), "Given invalid context.",
		return 1;
		);

	DASSERT(key != NULL, "Given invalid key.",
		return 1;
		);

	DASSERT((ctx->val_size == 0) || value != NULL, "Given invalid value.",
		return 1;
		);

	dvec vec = (dvec) bucket;

	int index = _dhtable_vector_search(ctx, vec, key);

	int t;
	if (index >= 0) {
		t = _dhtable_vector_replace(ctx, vec, value, index);
	} else {
		t = _dhtable_vector_push(ctx, vec, key, value);
	}

	return t;
}

/* Remove an element from
 * a vector.
 */
int dhtable_vector_rm(dhtable_ctx *ctx, void *bucket, void *key) {

	/* Verify ctx, key,
	 * get size, search,
	 * call remove, return.
	 */
	DASSERT(_dhtable_ctx_valid(ctx), "Given invalid context.",
		return 1;
		);

	DASSERT(key != NULL, "Given invalid key.",
		return 1;
		);

	dvec vec = (dvec) bucket;

	int t = _dhtable_vector_search(ctx, vec, key);

	if (t < 0)
		return 0;

	return dvec_rm(vec, t);
}

/* Join two vectors. */
int dhtable_vector_join(dhtable_ctx *ctx, void *dst, void *src) {

	/* Validate ctx,
	 * Get number of elements,
	 * for each element, search dst,
	 * if there, replace, else, put.
	 */
	DASSERT(_dhtable_ctx_valid(ctx), "Given invalid context.",
		return 1;
		);

	dvec dstvec = (dvec) dst;
	dvec srcvec = (dvec) src;

	size_t count = dvec_size(src);

	size_t i;
	for (i = 0; i < count; i++) {

		char *elem = dvec_get(srcvec, i);

		DASSERT(elem != NULL, "Failed to get element.",
			return 1;
			);

		void *key = (void*) elem;
		void *value = (void*) (elem + ctx->key_size);

		int index = _dhtable_vector_search(ctx, dstvec, key);

		int t;
		if (index < 0) {
			t = _dhtable_vector_push(ctx, dstvec, key, value);
		} else {
			t = _dhtable_vector_replace(ctx, dstvec, value, index);
		}

		if (t != 0)
			return 1;
	}

	return 0;
}
