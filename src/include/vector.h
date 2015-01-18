/** daelib/vector.h: Simple vector implementation.
 */

#ifndef __DAELIB_VECTOR_H
#define __DAELIB_VECTOR_H

/* Basic vector container.
 * It stores variable size containers.
 * You can find exacting detail in
 * vector.c. We assure continuity.
 */


/* size_t */
#include <stdlib.h>


/* Opaque structure. */
struct daelib_vector;

/* For sanity. */
typedef struct daelib_vector *dvec;


/* Iterators, opaque. */
typedef void *dvec_it;


/* Vector functions. */

/* Init/kill/copy. */
dvec dvec_init(size_t elem_size);
int  dvec_kill(dvec vec);
dvec dvec_copy(dvec vec);

/* Push/pop/peek. */
int   dvec_push(dvec vec, void *elem);
void *dvec_peek(dvec vec);
int   dvec_pop (dvec vec);

/* Size/metadata. */
size_t dvec_size     (dvec vec);
size_t dvec_elem_size(dvec vec);

/* Random access. */
void *dvec_get(dvec vec, size_t index);
int   dvec_put(dvec vec, void *elem, size_t index);
int   dvec_rm (dvec vec, size_t index);

/* Range operations. */
int dvec_join  (dvec dst, dvec src, size_t index);
int dvec_insert(dvec vec, size_t count, void *elem, size_t index);
int dvec_delete(dvec vec, size_t start, size_t end);

/* Iterators. */
dvec_it dvec_begin(dvec vec);
dvec_it dvec_end  (dvec vec);

dvec_it dvec_prev(dvec vec, dvec_it it);
dvec_it dvec_next(dvec vec, dvec_it it);

void   *dvec_iget(dvec vec, dvec_it it);


#endif // __DAELIB_VECTOR_H
