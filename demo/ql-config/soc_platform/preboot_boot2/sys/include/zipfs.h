#ifndef __ZIPFS_H__
#define __ZIPFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "flash.h"

typedef struct {
  uint64_t image_addr;
  uint64_t image_size;
} zipfs_entry_info_t;

typedef void *zipfs_handle_t;

zipfs_handle_t zipfs_open(const char *ptable_name);
int zipfs_find_entry_info(const zipfs_handle_t handle, const char *image_name,
                          zipfs_entry_info_t *info);
void zipfs_close(const zipfs_handle_t handle);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __ZIPFS_H__ */
