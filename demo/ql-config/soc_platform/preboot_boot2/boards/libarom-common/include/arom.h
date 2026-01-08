#ifndef _AROM_H_
#define _AROM_H_

#include <stddef.h>
#include "process.h"

struct process *arom_main_process(void);
int arom_ptable_init(const void *data, size_t size);

#endif /* _AROM_H_ */
