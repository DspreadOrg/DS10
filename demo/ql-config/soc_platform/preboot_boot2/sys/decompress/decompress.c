#include <stddef.h>
#include "decompress.h"
#include "lzma.h"

/*---------------------------------------------------------------------------*/
int
copy_sections(const struct load_table *p, uint8_t *base)
{
  while(p->vma) {
    size_t destLen = p->size;
    size_t srcLen = p->storSize;
    if(destLen == srcLen) {
      /* I'm sure src, dst and srcLen are all 4 bytes algined */
      uint32_t *src = (uint32_t *)(base + p->offset);
      uint32_t *dst = (uint32_t *)(p->vma);
      if(src != dst) {
        while(srcLen) {
          *dst++ = *src++;
          srcLen -= sizeof(*src);
        }
      }
    } else if(lzma_decode((uint8_t *)p->vma, &destLen, base + p->offset, &srcLen)) {
      return -1;
    }
    p++;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
