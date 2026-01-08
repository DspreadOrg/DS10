#ifndef __SHMRB_H__
#define __SHMRB_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned wptr;
    unsigned rptr;
    char *buf;
    unsigned size;
} shmrb_t;

/**
 * Initialize a shmrb.
 * rb     Datum to initialize.
 * buf    Buffer to use by rb.
 * size   size of (buf)
 */
static inline void shmrb_init (shmrb_t * rb, char *buf, unsigned size)
{
    rb->buf = buf;
    rb->size = size;
    rb->wptr = 0;
    rb->rptr = 0;
}

/**
 * Add a number of elements to the shmrb.
 * Return the number of elements actually added.
 */
size_t shmrb_add(shmrb_t *rb, const char *buf, size_t n);

/**
 * Read and remove a number of elements from the shmrb.
 * Return the number of elements actually read.
 */
size_t shmrb_get(shmrb_t *rb, char *buf, size_t n);

/**
 * Return available element in shmrb
 */
static inline size_t shmrb_get_avail(const shmrb_t *rb)
{
    int size = rb->wptr - rb->rptr;

    if (size < 0) {
        size += rb->size;
    }

    return (size_t)size;
}

/**
 * Return available space in shmrb
 */
static inline size_t shmrb_get_free(const shmrb_t *rb)
{
    return (size_t)((rb->size - 1) - shmrb_get_avail(rb));
}

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* __SHMRB_H__ */
