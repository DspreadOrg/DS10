#ifndef _DECOMPRESS_H_
#define _DECOMPRESS_H_

#include <stdint.h>

struct load_table {
  uint32_t vma;
  uint32_t size;
  uint32_t offset;
  uint32_t storSize;
};
/*---------------------------------------------------------------------------*/
int copy_sections(const struct load_table *p, uint8_t *base);
/*---------------------------------------------------------------------------*/
#endif
