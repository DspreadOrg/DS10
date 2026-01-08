#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <stdint.h>

typedef struct {
  unsigned wptr;
  unsigned rptr;
  char *buf;
  unsigned size;
}ringbuffer_t;
/*---------------------------------------------------------------------------*/
/*
 * Initialize a ringbuffer.
 * rb     Datum to initialize.
 * buf    Buffer to use by rb.
 * size   size of (buf)
 */

static inline void
ringbuffer_init(ringbuffer_t *rb, char *buf, unsigned size)
{
  rb->buf = buf;
  rb->size = size;
  rb->wptr = 0;
  rb->rptr = 0;
}
/*
 * Add a number of elements to the ringbuffer.
 * Return the number of elements actually added.
 */
unsigned ringbuffer_add(ringbuffer_t *rb, const char *buf, unsigned n);
/*
 * Read and remove a number of elements from the ringbuffer.
 * Return the number of elements actually read.
 */
unsigned ringbuffer_get(ringbuffer_t *rb, char *buf, unsigned n);
/*
 * Return available element in ringbuffer
 */
static inline unsigned
ringbuffer_get_data_size(const ringbuffer_t *rb)
{
  return (rb->wptr + rb->size - rb->rptr) % rb->size;
}
/*
 * Return available space in ringbuffer
 */
static inline unsigned
ringbuffer_get_free_size(const ringbuffer_t *rb)
{
  return (rb->size - 1) - ringbuffer_get_data_size(rb);
}
/*---------------------------------------------------------------------------*/
#endif
