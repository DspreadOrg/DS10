#include <string.h>
#include <stdbool.h>
#include "ptable.h"
#include "heapmem.h"
#include "crc32.h"
#include "flash.h"

#include "log.h"
#define LOG_MODULE "PTable"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
static struct ptable *ptable_info;
/*---------------------------------------------------------------------------*/
int
ptable_init(const void *data, size_t size)
{
  struct ptable *ptable = (struct ptable *)data;
  size_t ptable_size;
  bool no_crc = false;

  if(ptable->magic != PTABLE_MAGIC) {
    LOG_ERR("[%s] Invalid magic id = 0x%x\n", __func__, ptable->magic);
    return -1;
  }

  ptable_size = sizeof(*ptable);
  if(size < ptable_size) {
    if(size < ptable_size - sizeof(ptable->crc)) {
      printf("[%s] error: ptable size too small = 0x%x\n", __func__, size);
      return -1;
    } else {
      no_crc = true;
    }
  }
  if(!no_crc) {
    unsigned crc = 0 ^ ~0;
    crc = crc32(crc, ptable, ptable_size - sizeof(ptable->crc));
    crc = crc ^ ~0;
    if(ptable->crc != crc) {
      printf("[%s] error: ptable crc not match(%08x, %08x)\n", __func__, ptable->crc, crc);
      return -1;
    }
  }

  if(ptable->count > MAX_PTABLE_PARTS) {
    LOG_ERR("[%s] ptable->count > MAX_PTABLE_PARTS\n", __func__);
    return -1;
  }

  ptable_size = sizeof(struct ptable) - sizeof(ptable->crc) -
    (MAX_PTABLE_PARTS - ptable->count) * sizeof(struct ptentry);

  /* save a partition table copy */
  if(ptable_info) {
    heapmem_free(ptable_info);
  }
  ptable_info = (struct ptable *)heapmem_alloc(ptable_size);
  if(!ptable_info) {
    LOG_ERR("Out of memory\n");
    return -1;
  }
  memcpy(ptable_info, data, ptable_size);

  return 0;
}
/*---------------------------------------------------------------------------*/
void
ptable_dump(void)
{
  struct ptentry *ptn;
  unsigned i;

  for(i = 0; i < ptable_info->count; ++i) {
    ptn = &ptable_info->parts[i];
    LOG_DBG("Ptn %d name='%s' start=%08x size=%08x type=%s depth=%d\n", i, ptn->name,
            ptn->start, ptn->size, ptn->type, ptn->depth);
  }
}
/*---------------------------------------------------------------------------*/
struct ptentry *
ptable_find_entry(const char *name)
{
  struct ptentry *ptn;
  unsigned i;

  for(i = 0; i < ptable_info->count; ++i) {
    ptn = &ptable_info->parts[i];
    if(!strcmp(ptn->name, name)) {
      return ptn;
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
struct ptentry *
ptable_find_flash_entry(const char *name)
{
  struct ptentry *flash_entry = NULL;
  struct ptentry *ptn;
  unsigned i;

  for(i = 0; i < ptable_info->count; ++i) {
    ptn = &ptable_info->parts[i];
    if(!strncmp(ptn->type, PTABLE_TYPE_FLASH, sizeof(PTABLE_TYPE_FLASH))) {
      flash_entry = ptn;
    }
    if(!strcmp(ptn->name, name)) {
      return flash_entry;
    }
  }

  return flash_entry;
}
/*---------------------------------------------------------------------------*/
/*
 * char flash_type[16] is divided to two strings. [type string] + [flash id string]
 * if flash0: "flash""0"
 * if flash1: "flash""1"
 */
int
ptable_get_flash_id(struct ptentry *ptn)
{
  char *ptr = ptn->type;
  if(strncmp(ptr, PTABLE_TYPE_FLASH, sizeof(PTABLE_TYPE_FLASH))) {
    return -1;
  }

  ptr += strlen(PTABLE_TYPE_FLASH) + 1;
  if(strlen(ptr) > 0) {
    return atoi(ptr);
  }

  /* for compatiable with old version */
  if(!strcmp(ptn->name, PTABLE_FLASH_EXTERNAL)) {
    return FLASH_ID_EXTERNAL;
  } else if(!strcmp(ptn->name, PTABLE_FLASH_INTERNAL)) {
    return FLASH_ID_INTERNAL;
  }

  return -1;
}
/*---------------------------------------------------------------------------*/
void
ptable_exit(void)
{
  if(ptable_info) {
    heapmem_free(ptable_info);
  }
  ptable_info = NULL;
}
/*---------------------------------------------------------------------------*/
struct ptable *
ptable_get(void)
{
  return ptable_info;
}
/*---------------------------------------------------------------------------*/
