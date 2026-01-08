#ifndef __SYS_CLOSE_ALL_H
#define __SYS_CLOSE_ALL_H

#include <stdint.h>

typedef struct {
  uintptr_t pc;
  uint32_t spsr;
} transfer_parameter;

void sys_close_all(void);
__attribute__ ((noreturn, naked, target("arm")))
void transfer_control(transfer_parameter *param);

#endif /* __SYS_CLOSE_ALL_H */
