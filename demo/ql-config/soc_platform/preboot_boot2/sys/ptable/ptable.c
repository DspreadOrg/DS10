#include <string.h>
#include <stdbool.h>
#include "ptable.h"
#include "heapmem.h"
#include "crc32.h"
#include "flash.h"
#ifdef USE_EMMC_FLASH
#include "emmc.h"
#endif

#include "log.h"
#define LOG_MODULE "PTable"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
static struct ptable *ptable_info;
/*---------------------------------------------------------------------------*/
extern unsigned char __scratch2_start, __scratch2_end;
/*---------------------------------------------------------------------------*/
#ifdef USE_EMMC_FLASH
/*---------------------------------------------------------------------------*/
static void
adjust_emmc_area_size(struct ptentry *start, struct ptentry *end, int64_t adjustment)
{
  struct ptentry *entry = end;
  uint8_t depth = 0;

  /* adjust the last sub-partition size */
  while(--entry >= start) {
    if(!depth || entry->depth == depth - 1) {
      if(entry->size == entry->vsize) {
        entry->size += adjustment;
        entry->vsize = entry->size;
      } else {
        entry->size += adjustment;
      }
      depth = entry->depth;
    }
  }

  /* adjust emmc flash size */
  entry = ptable_find_flash_entry(start->name);
  entry->size += adjustment;
  entry->vsize = entry->size;
  /* adjust all group flash size */
  entry = &ptable_info->parts[0];
  entry->size += adjustment;
  entry->vsize = entry->size;
}
/*---------------------------------------------------------------------------*/
static void
adjust_emmc_ptable_size(void)
{
  struct ptentry *entry = &ptable_info->parts[0];
  struct ptentry *end = &ptable_info->parts[ptable_info->count];
  struct ptentry *start = NULL;
  uint8_t depth = 0;
  int64_t adjustment = 0;

  while(entry < end) {
    if(!strcmp(entry->type, PTABLE_TYPE_AREA)) {
      depth = entry->depth;
      uint64_t area_size = 0;
      if(!strcmp(entry->name, "user")) {
        area_size = emmc_area_size(EMMC_AREA_USER);
      } else if(!strcmp(entry->name, "boot1")) {
        area_size = emmc_area_size(EMMC_AREA_BOOT1);
      } else if(!strcmp(entry->name, "boot2")) {
        area_size = emmc_area_size(EMMC_AREA_BOOT2);
      }
      if(start) {
        /* adjust the area and sub-partition size */
        adjust_emmc_area_size(start, entry, adjustment);
      }
      /* save for next loop use */
      start = entry;
      adjustment = area_size - entry->size;
    } else if(entry->depth < depth) {
      /*
       * assert(start != NULL)
       * process the last area and not the last partition
       */
      adjust_emmc_area_size(start, entry, adjustment);
      start = NULL; /* to inform no need more process */
      break;
    }
    entry++;
  }
  if(start) {
    /*
     * process the last area and the last partition
     */
    adjust_emmc_area_size(start, entry, adjustment);
  }
}
#endif
/*---------------------------------------------------------------------------*/
int
ptable_init(const void *data, size_t size)
{
  struct ptable *ptable = (struct ptable *)data;
  size_t ptable_size;

  if(ptable->magic == PTABLE_MAGIC_A) {
    bool no_crc = false;
    struct ptable_a *ptable = (struct ptable_a *)data;
    ptable_size = sizeof(*ptable);

    if(size < ptable_size) {
      if(size < ptable_size - sizeof(ptable->crc)) {
        LOG_ERR("[%s] error: ptable size too small = 0x%x\n",
                __func__, size);
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
        LOG_ERR("[%s] error: ptable crc not match(%08lx, %08x)\n",
                __func__, ptable->crc, crc);
        return -1;
      }
    }

    if(ptable->count > MAX_PTABLE_PARTS) {
      LOG_ERR("[%s] ptable->count > MAX_PTABLE_PARTS\n", __func__);
      return -1;
    }
    ptable_size = sizeof(struct ptable) + ptable->count * sizeof(struct ptentry);
  } else if(ptable->magic == PTABLE_MAGIC) {
    ptable_size = sizeof(struct ptable) + ptable->count * sizeof(struct ptentry);
    if(size < ptable_size) {
      LOG_ERR("[%s] error: ptable size too small = 0x%x\n", __func__, size);
      return -1;
    }
  } else {
    LOG_ERR("[%s] Invalid magic id = 0x%lx\n", __func__, ptable->magic);
    return -1;
  }

  /* save a partition table copy */
  if(ptable_info) {
    heapmem_free(ptable_info);
  }
  ptable_info = (struct ptable *)heapmem_alloc(ptable_size);
  if(!ptable_info) {
    LOG_ERR("Out of memory\n");
    return -1;
  }

  bool update_crc = false;

  if(ptable->magic == PTABLE_MAGIC_A) {
    struct ptable_a *ptable = (struct ptable_a *)data;
    ptable_info->magic = PTABLE_MAGIC;
    ptable_info->count = ptable->count;
    for(uint32_t i = 0; i < ptable->count; i++) {
      memcpy(ptable_info->parts[i].name, ptable->parts[i].name,
             MAX_PTABLE_ENTRY_NAME);
      memcpy(ptable_info->parts[i].type, ptable->parts[i].type,
             MAX_PTABLE_ENTRY_TYPE);
      ptable_info->parts[i].start = ptable->parts[i].start == (uint32_t)-1
        ? (uint64_t)-1 : ptable->parts[i].start;
      ptable_info->parts[i].size = ptable->parts[i].size == (uint32_t)-1
        ? (uint64_t)-1 : ptable->parts[i].size;
      ptable_info->parts[i].vstart = ptable->parts[i].vstart == (uint32_t)-1
        ? (uint64_t)-1 : ptable->parts[i].vstart;
      ptable_info->parts[i].vsize = ptable->parts[i].vsize == (uint32_t)-1
        ? (uint64_t)-1 : ptable->parts[i].vsize;
      ptable_info->parts[i].depth = ptable->parts[i].depth == (uint32_t)-1
        ? (uint64_t)-1 : ptable->parts[i].depth;
    }
    update_crc = true;
  } else {
    memcpy(ptable_info, data, ptable_size);
  }

#ifdef USE_EMMC_FLASH
  adjust_emmc_ptable_size();
  update_crc = true;
#endif

  if(update_crc) {
    ptable_info->crc = 0;
    unsigned crc = 0 ^ ~0;
    crc = crc32(crc, ptable_info, ptable_size);
    crc = crc ^ ~0;
    ptable_info->crc = crc;
  }

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
    LOG_DBG("Ptn %d name='%s' start=%010llx size=%010llx vsize=%010llx type=%s depth=%d\n",
            i, ptn->name, ptn->start, ptn->size, ptn->vsize, ptn->type, ptn->depth);
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
ptable_find_entry_with_type(const char *type)
{
  struct ptentry *ptn;
  unsigned i;

  for(i = 0; i < ptable_info->count; ++i) {
    ptn = &ptable_info->parts[i];
    if(!strcmp(ptn->type, type)) {
      return ptn;
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
static struct ptentry *
_ptable_find_parent_entry_with_type(const char *name, const char *type)
{
  struct ptentry *entry = NULL;
  struct ptentry *ptn;
  unsigned i;

  for(i = 0; i < ptable_info->count; ++i) {
    ptn = &ptable_info->parts[i];
    if(!strcmp(ptn->type, type)) {
      entry = ptn;
    }
    if(!strcmp(ptn->name, name)) {
      return entry;
    }
  }

  return NULL;
}
/*---------------------------------------------------------------------------*/
struct ptentry *
ptable_find_flash_entry(const char *name)
{
  return _ptable_find_parent_entry_with_type(name, PTABLE_TYPE_FLASH);
}
/*---------------------------------------------------------------------------*/
struct ptentry *
ptable_find_area_entry(const char *name)
{
  return _ptable_find_parent_entry_with_type(name, PTABLE_TYPE_AREA);
}
/*---------------------------------------------------------------------------*/
/*
 * char flash_type[16] is divided to two strings. [type string] + [flash id string]
 * if flash0: "flash""0"
 * if flash1: "flash""1"
 */
int
ptable_get_flash_id(const struct ptentry *ptn)
{
  const char *ptr = ptn->type;
  if(strncmp(ptr, PTABLE_TYPE_FLASH, sizeof(PTABLE_TYPE_FLASH))) {
    return -1;
  }

  ptr += strlen(PTABLE_TYPE_FLASH) + 1;
  if(strlen(ptr) > 0) {
    return atoi(ptr);
  }

  /* for compatiable with old version */
  if(!strcmp(ptn->name, PTABLE_FLASH_EXTERNAL)) {
    return FLASH_ID_FIRST;
  } else if(!strcmp(ptn->name, PTABLE_FLASH_INTERNAL)) {
    return FLASH_ID_SECOND;
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
size_t
ptable_size(const struct ptable *ptable)
{
  return sizeof(*ptable) + ptable->count * sizeof(struct ptentry);
}
/*---------------------------------------------------------------------------*/
int
ptable_backup(void *data, unsigned sz)
{
#if defined BOARD_SC2 || defined BOARD_JACANA || defined BOARD_JACANA_RISCV
  (void)data;
  (void)sz;
  return 0;
#else
  uint8_t *stash;
  ssize_t stash_size;
  int ret;

  /*
   * write ptable to __scratch2_start
   */
  stash = &__scratch2_start;
  stash_size = &__scratch2_end - &__scratch2_start;
  if(sz > (unsigned)stash_size) {
    ret = -1;
  } else {
    memcpy(stash, data, sz);
    ret = 0;
  }

  return ret;
#endif
}
/*---------------------------------------------------------------------------*/
