#ifndef _LZMA_H_
#define _LZMA_H_

#include <stdlib.h>
#include <stdint.h>

/* ---------- LZMA Properties ---------- */
typedef struct {
  uint8_t lc;
  uint8_t lp;
  uint8_t pb;
  uint8_t _pad_;
  uint32_t dicSize;
} lzma_props_t;

int lzmaprops_decode(lzma_props_t *p, const uint8_t *data, size_t size);
int lzma_decode(uint8_t *dest, size_t *destLen, uint8_t *src, size_t *srcLen);

#endif
