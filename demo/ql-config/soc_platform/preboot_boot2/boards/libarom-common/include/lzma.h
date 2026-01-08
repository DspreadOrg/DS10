#ifndef _LZMA_H_
#define _LZMA_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ISzAlloc ISzAlloc;
typedef const ISzAlloc *ISzAllocPtr;
typedef void *LzmaHandle;

struct ISzAlloc {
  void *(*Alloc)(ISzAllocPtr p, size_t size);
  void (*Free)(ISzAllocPtr p, void *address); /* address can be 0 */
};

int lzma_decode_init(uint8_t *dest, size_t *destLen,
                     const uint8_t *src, size_t *srcLen,
                     ISzAllocPtr allocPtr, LzmaHandle *handle);
int lzma_decode_loop(LzmaHandle handle, size_t *destLen, size_t *srcLen);
void lzma_decode_exit(LzmaHandle handle);
bool lzma_decode_magic(const void *data, size_t size);
int lzma_decode(uint8_t *dest, size_t *destLen,
                const uint8_t *src, size_t *srcLen);

#endif
