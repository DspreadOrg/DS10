#ifndef _AROM_CRANE_H_
#define _AROM_CRANE_H_

#include <stddef.h>
#include "process.h"

/* boot2 local event */
typedef enum {
  DO_NORMAL_BOOT,
  DO_PRODUCTION_BOOT,
  DO_CONFIG_FOR_DOWNLOAD,
} bootloader_ev_t;

struct process *arom_main_process(void);
int arom_ptable_init(const void *data, size_t size);

#endif /* _AROM_CRANE_H_ */
