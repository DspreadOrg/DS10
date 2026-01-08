#ifndef __PTABLE_H__
#define __PTABLE_H__

#include <stddef.h>

#define MAX_PTABLE_ENTRY_NAME   32
#define MAX_PTABLE_ENTRY_TYPE   16
#define MAX_PTABLE_PARTS        32
#define PTABLE_MAGIC            0x41505442
#define QSPI_FLASH_BASE         0x80000000

#define PTABLE_TYPE_RAW         "raw"
#define PTABLE_TYPE_UBI         "ubi"
#define PTABLE_TYPE_CUST        "cust"
#define PTABLE_TYPE_PART        "part"
#define PTABLE_TYPE_FLASH       "flash"
#define PTABLE_TYPE_GROUP       "group"

#define PTABLE_FLASH_EXTERNAL   "external"
#define PTABLE_FLASH_INTERNAL   "internal"

/*
 * @name: partition name, e.g. bootloader, radio
 * @type: partition type, e.g. ubi or raw
 * @start: partition start address within flash
 * @size: partition size in bytes
 * @vstart: partition image load address at runtime
 * @vsize: partition image real size in bytes
 * @depth: partition nested depth
 */
struct ptentry {
  char name[MAX_PTABLE_ENTRY_NAME];
  char type[MAX_PTABLE_ENTRY_TYPE];
  unsigned start;
  unsigned size;
  unsigned vstart;
  unsigned vsize;
  unsigned depth;
};

/*
 * @magic: ptable magic id
 * @count: partition numbers
 * @parts: info for each partition entry
 */
struct ptable {
  unsigned magic;
  unsigned count;
  struct ptentry parts[MAX_PTABLE_PARTS];
  unsigned crc;
};

/* tools to populate and query the partition table */
int ptable_init(const void *data, size_t size);
struct ptentry *ptable_find_entry(const char *name);
struct ptentry *ptable_find_flash_entry(const char *name);
int ptable_get_flash_id(struct ptentry *ptn);
struct ptable *ptable_get(void);
void ptable_dump(void);
void ptable_exit(void);

#endif /* __PTABLE_H__ */
