#ifndef IPC_H
#define IPC_H

#include <stdint.h>
#include "cpu.h"
#include "reg.h"

#define IPC_FLAGS_POLLING_MODE         (1 << 0)
#define IPC_FLAGS_IRQ_MODE             (1 << 1)
#define IPC_LINE_MAX                   16
/*---------------------------------------------------------------------------*/
typedef void (*ipc_handler_t)(void *);
/*---------------------------------------------------------------------------*/
typedef enum {
  IPC_DEV_NUM_0,
  IPC_DEV_NUM_MAX,
} ipc_dev_num_t;
/*---------------------------------------------------------------------------*/
typedef struct {
  uint32_t isrr;
  uint32_t wdr;
  uint32_t isrw;
  uint32_t icr;
  uint32_t iir;
  uint32_t rdr;
} ipc_reg_t;
/*---------------------------------------------------------------------------*/
typedef struct {
  ipc_reg_t *regs;
  uint32_t clk_ctrl;
  int irq;
  uint32_t flags;
  struct process *ipc_process;
  ipc_handler_t handler[IPC_LINE_MAX];
} ipc_dev_t;
/*---------------------------------------------------------------------------*/
int ipc_init(ipc_dev_num_t dev, uint32_t flags);
int ipc_register_line_handler(ipc_dev_num_t dev, int line, ipc_handler_t handler);
int ipc_trigger(ipc_dev_num_t dev, int line);
int ipc_exit(ipc_dev_num_t dev);
/*---------------------------------------------------------------------------*/

#endif
