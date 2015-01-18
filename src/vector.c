/** daelib/vector.c: Simple vector implementation.
 * TODO:
 * - growth ratio
 * - further profiling
 * - SizeMax + asserts
 */

#define _VECTOR_DEBUG

/* Prototypes. */
#include "vector.h"

/* Assertions. */
#include "assert.h"

/* malloc(), realloc(), free(). */
#include <stdlib.h>

/* memcpy(), memmove(). */
#include <string.h>


/* Base definition for a vector. */
struct daelib_vector {
	size_t elem_size;
	size_t elem_count;

	size_t allocated;
	void *data;
};


#if 0 /* For debugging. */
void _dvec_print(dvec vec);
void _dvec_print_int_map(dvec vec);
#endif // 0


/* Smears bits to the right.
 * Round up to the next 2^n-1.
 * Returns -1 on negative.
 */
static int _dvec_smear(int num) {

	/* Return 2^n - 1, where n is the
	 * largest bitplace with an on bit.
	 * (AKA round up to the next 2^n-1.)
	 */
	int tmp = num;
	tmp |= tmp >>  1;
	tmp |= tmp >>  2;
	tmp |= tmp >>  4;
	tmp |= tmp >>  8;
	tmp |= tmp >> 16;
	return tmp;
}

/* Round up a vector's size. */
static int _dvec_round(size_t size) {

	/* Adjust smear (2^n-1), add one (2^n),
	 * and make it make sure it won't round up
	 * powers of two.
	 */
	return _dvec_smear(size - 1) + 1;
}

/* Resize a vector.
 * Returns nonzero on error.
 * ASSUMES VEC IS VALID.
 * TODO: 1.5 growth factor?
 */
static int _dvec_resize(dvec vec, size_t newsize) {

	/* Heuristically eliminate most queries.
	 * Verify we won't go over our type limits.
	 * Find the desired rounded size.
	 * Realloc (or malloc) a new (appropriately
	 * sized) data block. Check for errors
	 * and return.
	 */
	if ((newsize * vec->elem_size <= vec->allocated) &&
	    newsize * vec->elem_size > ((vec->allocated >> 2)))
		return 0;

	size_t new_allocated = _dvec_round(newsize * vec->elem_size);

	void *new_data = NULL;
	new_data = (void*) realloc(vec->data, new_allocated);

	DASSERT(new_data != NULL || new_allocated == 0,
	        "Failed to realloc vector.",
		return 1;
		);

	vec->data = new_data;
	vec->allocated = new_allocated;

	return 0;
}

/* Determine if a vector is valid.
 * If valid return nonzero. Else return 0.
 */
static int _dvec_valid(dvec vec) {

	/* Check that the pointer is valid,
	 * that you have enough allocated memory,
	 * and that you (if you have allocated memory)
	 * the data is not NULL.
	 */
	if (vec == NULL)
		return 0;

	if (vec->elem_size * vec->elem_count > vec->allocated)
		return 0;

	if (vec->data == NULL &&
	    vec->allocated != 0)
		return 0;

	return 1;
}

/* Insert the contents of a buffer
 * into a vector. Returns nonzero on
 * error. Will not corrupt on failure.
 * ASSUMES VECTOR AND INDEX IS VALID.
 */
static int _dvec_insert(dvec vec, size_t count, void *elems, size_t index) {

	/* Resize, move memory, copy memory,
	 * adjust vec->count, return.
	 */
	size_t new_count = vec->elem_count + count;

	if (_dvec_resize(vec, new_count) != 0)
		return 1;

	if (index != vec->elem_count)
		memmove((char*) (vec->data) + (index + count) * vec->elem_size,
		        (char*) (vec->data) + (index) * vec->elem_size,
		        (vec->elem_count - index) * vec->elem_size);

	memcpy((char*) (vec->data) + (index) * vec->elem_size,
	       (char*) elems,
	       count * vec->elem_size);

	vec->elem_count = new_count;

	return 0;
}

/* Delete a range of the buffer.
 * Assumes vector is valid.
 * Deletes [start, end).
 * Cannot fail.
 */
static void _dvec_delete(dvec vec, size_t start, size_t end) {

	/* Resize, move memory if not pop,
	 * adjust vec->elem_count
	 * return.
	 */
	size_t new_count = vec->elem_count - (end - start);

	if (end != vec->elem_count)
		memmove(((char*) (vec->data)) + (start) * vec->elem_size,
			((char*) (vec->data)) + (end) * vec->elem_size,
			(vec->elem_count - end) * vec->elem_size);

	_dvec_resize(vec, new_count);

	vec->elem_count -= (end - start);
}

/* Create a new vector.
 * Returns NULL on error.
 */
dvec dvec_init(size_t elem_size) {

	/* Allocate a vector structure,
	 * check for failures, apply
	 * defaults.
	 */
	dvec new_vec = (dvec) malloc(sizeof(struct daelib_vector));

	DASSERT(new_vec != NULL, "Failed to allocate new vector.",
		return NULL;
		);

	new_vec->elem_size = elem_size;
	new_vec->elem_count = 0;
	new_vec->allocated = 0;
	new_vec->data = NULL;

	return new_vec;
}

/* Free a vector.
 * Returns nonzero on error.
 * No dvec functions will
 * accept the free'd vector.
 */
int dvec_kill(dvec vec) {

	/* Check if vector is valid,
	 * free non-NULL data, invalidate
	 * fields, free the struct.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 1;
		);

	if (vec->data != NULL)
		free(vec->data);

	vec->allocated = 1;
	vec->data = NULL;

	free(vec);

	return 0;
}

/* Copy a vector.
 * Returns NULL on error.
 */
dvec dvec_copy(dvec vec) {

	/* Check if vec is valid,
	 * allocate vector, ?(allocate
	 * memory, copy memory,) return.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return NULL;
		);

	dvec t= (dvec) malloc(sizeof(struct daelib_vector));

	DASSERT(t != NULL, "Failed to allocate new vector.",
		return NULL;
		);

	t->data = NULL;
	t->allocated = vec->allocated;
	t->elem_count = vec->elem_count;
	t->elem_size = vec->elem_size;

	if (t->allocated == 0)
		return t;

	t->data = malloc(vec->allocated);

	DASSERT(t->data != NULL, "Failed to allocate new vector data.",
		free(t);
		return NULL;
		);

	memcpy(t->data, vec->data, t->elem_count * t->elem_size);

	return t;
}

/* Push an element to the back
 * of a vector. Returns nonzero
 * on error.
 */
int dvec_push(dvec vec, void *elem) {

	/* Check the validity of the
	 * vector, call _dvec_insert,
	 * return.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 1;
		);

	return _dvec_insert(vec, 1, elem, vec->elem_count);
}

/* Peek the last element of
 * a vector. Returs NULL on
 * error.
 */
void *dvec_peek(dvec vec) {

	/* Check if vector is valid,
	 * check if size > 0,
	 * return the last element.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return NULL;
		);

	DASSERT(vec->elem_count != 0, "No elements to peek.",
		return NULL;
		);

	return (char*) (vec->data) +
		(vec->elem_count - 1) * (vec->elem_size);
}

/* Pop the last element off
 * of a vector. Returns nonzero
 * on error.
 */
int dvec_pop(dvec vec) {

	/* Check if vector is valid,
	 * Check if size > 0,
	 * delete range, return.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 1;
		);

	DASSERT(vec->elem_count > 0, "No elements to pop.",
		return 1;
		);

	_dvec_delete(vec, vec->elem_count - 1, vec->elem_count);

	return 0;
}

/* Return the number of elements
 * in a vector. Returns 0 on error.
 */
size_t dvec_size(dvec vec) {

	/* Check if vector is valid,
	 * return the number of elements.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 0;
		);

	return vec->elem_count;
}

/* Return the size of an
 * element. Returns 0 on error.
 */
size_t dvec_elem_size(dvec vec) {

	/* Check if the vector is valid,
	 * return the element size.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 0;
		);

	return vec->elem_size;
}

/* Get a random element of a vector.
 * Returns NULL on error.
 */
void *dvec_get(dvec vec, size_t index) {

	/* Check that vector is valid,
	 * check that the index is good,
	 * return the data.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return NULL;
		);

	DASSERT(index < vec->elem_count, "Index is out of bounds.",
		return NULL;
		);

	return (void*) ((char*) (vec->data) + (index * vec->elem_size));
}

/* Insert an element at an arbitrary
 * location. Returns nonzero on error.
 */
int dvec_put(dvec vec, void *elem, size_t index) {

	/* Check that vector is valid,
	 * check that index is valid,
	 * call dvec_insert.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 1;
		);

	DASSERT(index > vec->elem_count, "Index is out of bounds.",
		return 1;
		);

	return _dvec_insert(vec, 1, elem, index);
}

/* Delete a single element.
 * Returns nonzero on error.
 */
int dvec_rm(dvec vec, size_t index) {

	/* Check that vector is valid,
	 * check that index is valid,
	 * call _dvec_delete.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return 1;
		);

	DASSERT(index < vec->elem_count, "Index is out of bounds.",
		return 1;
		);

	_dvec_delete(vec, index, index+1);

	return 0;
}

/* Insert the contents of a vector
 * into another. Returns nonzero on
 * error. Will not corrupt on failure.
 */
int dvec_join(dvec dst, dvec src, size_t index) {

	/* Check that both vectors are valid,
	 * check that they are compatable,
	 * call _dvec_insert.
	 */
	DASSERT(_dvec_valid(dst), "Given invalid destination vector.",
		return 1;
		);

	DASSERT(_dvec_valid(src), "Given invalid source vector.",
		return 1;
		);

	DASSERT(dst->elem_size == src->elem_size, "Element sizes do not match.",
		return 1;
		);

	DASSERT(index <= dst->elem_count, "Index is out of bounds.",
		return 1;
		);

	return _dvec_insert(dst, src->elem_count, src->data, index);
}

/* Insert the contents of a buffer
 * into a vector. Returns nonzero on
 * error. Will not corrupt on failure.
 */
int dvec_insert(dvec dst, size_t count, void *elems, size_t index) {

	/* Check if the vector is valid,
	 * verify index, make room, shuffle
	 * around, copy, adjust, return.
	 */
	DASSERT(_dvec_valid(dst), "Given invalid vector.",
	        return 1;
		);

	DASSERT(index <= dst->elem_count, "Index is out of bounds.",
		return 1;
		);

	return _dvec_insert(dst, count, elems, index);
}

/* Delete a portion of a vector.
 * Returns nonzero on error.
 * Will not corrupt on failure.
 */
int dvec_delete(dvec dst, size_t start, size_t end) {

	/* Verify vector, indicies,
	 * call remove, return.
	 */
	DASSERT(_dvec_valid(dst), "Given invalid vector.",
		return 1;
		);

	DASSERT(end <= dst->elem_count,	"End is out of bounds.",
		return 1;
		);
	DASSERT(start <= end, "Start comes after end.",
		return 1;
		);

	_dvec_delete(dst, start, end);

	return 0;
}

/* Return first index.
 * if empty, return NULL.
 */
dvec_it dvec_begin(dvec vec) {

	/* Verify vector, verify
	 * size, return first index.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return (dvec_it) NULL;
		);

	if (vec->elem_count == 0)
		return (dvec_it) NULL;

	return (dvec_it) 0 + 1;
}

/* Return final index.
 * if empty, return NULL.
 */
dvec_it dvec_end(dvec vec) {

	/* Verify vector, verify
	 * size, return last index.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return (dvec_it) NULL;
		);

	if (vec->elem_count == 0)
		return (dvec_it) NULL;

	return (dvec_it) vec->elem_count;
}


/* Get the next index.
 * If at last index, return NULL.
 * If at NULL, return first index.
 */
dvec_it dvec_next(dvec vec, dvec_it it) {

	/* Verify vector, iterator,
	 * increment iterator, if at end,
	 * invalidate, return.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return (dvec_it) NULL;
		);

	size_t i = (size_t) it;

	DASSERT(i <= vec->elem_count, "Given invalid iterator.",
		return (dvec_it) NULL;
		);

	i++;

	if (i > vec->elem_count)
		i = 0;

	return (dvec_it) i;
}

/* Get the previous index.
 * If at first index, return NULL.
 * If at NULL, return last index.
 */
dvec_it dvec_prev(dvec vec, dvec_it it) {

	/* Verify vector, iterator,
	 * decrement iterator, if at invalid,
	 * go to last index.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector.",
		return (dvec_it) NULL;
		);

	size_t i = (size_t) it;

	DASSERT(i <= vec->elem_count, "Given invalid iterator.",
		return (dvec_it) NULL;
		);

	i--;

	if (i > vec->elem_count)
		i = vec->elem_count;

	return (dvec_it) i;
}

/* Get the element at it. */
void *dvec_iget(dvec vec, dvec_it it) {

	/* Verify vector, iterator,
	 * get element.
	 */
	DASSERT(_dvec_valid(vec), "Given invalid vector",
		return NULL;
		);

	size_t index = (size_t) it - 1;

	DASSERT(index < vec->elem_count, "Given invalid iterator.",
		return NULL;
		);

	return (void*) ((char*) (vec->data) + (index * vec->elem_size));
}


#if 0

/* Debugging.
 * Will not fault.
 */
void _dvec_print(dvec vec) {

	printf("Vector (%p): ", vec);

	if (!_dvec_valid(vec)) {
		printf("invalid, ");
	}

	if (vec == NULL) {
		printf("NULL.\n");
		return;
	}

	printf("elem_count: %d, elem_size: %d, allocated: %d, data: %p.\n",
	       (int) vec->elem_count, (int) vec->elem_size, (int) vec->allocated, vec->data);

	return;
}

void _dvec_print_int_map(dvec vec) {

	printf("Vector [%p]: [", vec);

	int i;
	for (i = 0; i < vec->elem_count; i++) {
		int *elem = &((int*)vec->data)[i * 2];
		printf("%d:%d", *elem, *(elem + 1));
		if (i < vec->elem_count - 1)
			printf(", ");
		else
			printf("].\n");
	}
}

#endif // 0
