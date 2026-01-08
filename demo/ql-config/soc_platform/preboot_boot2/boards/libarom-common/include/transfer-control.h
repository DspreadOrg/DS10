#ifndef _TRANSFER_CONTROL_H_
#define _TRANSFER_CONTROL_H_

#include <stdint.h>
#include "syscall-arom.h"

/*---------------------------------------------------------------------------*/
#define SYSCALL_SVC_TRANSFER_CONTROL (SYSCALL_MAX + 1)
/*---------------------------------------------------------------------------*/
#ifdef __ARM_ARCH_ISA_ARM
/* For Cortex-R/A */
#define MODE32_svc                   0x13
#define SPSR_T_ARM                   0
#define SPSR_E_LITTLE                0
#define SPSR_FIQ_BIT                 (1 << 0)
#define SPSR_IRQ_BIT                 (1 << 1)
#define SPSR_ABT_BIT                 (1 << 2)
#define DISABLE_ALL_EXCEPTIONS       (SPSR_FIQ_BIT | SPSR_IRQ_BIT | SPSR_ABT_BIT)

#define MODE_RW_SHIFT                0x4
#define MODE_RW_32                   0x1
#define MODE32_SHIFT                 0
#define MODE32_MASK                  0x1f
#define SPSR_T_SHIFT                 5
#define SPSR_T_MASK                  0x1
#define SPSR_E_SHIFT                 9
#define SPSR_E_MASK                  0x1
#define SPSR_AIF_SHIFT               6
#define SPSR_AIF_MASK                0x7

#define SPSR_MODE32(mode, isa, endian, aif) \
  (MODE_RW_32 << MODE_RW_SHIFT | \
   ((mode) & MODE32_MASK) << MODE32_SHIFT | \
   ((isa) & SPSR_T_MASK) << SPSR_T_SHIFT | \
   ((endian) & SPSR_E_MASK) << SPSR_E_SHIFT | \
   ((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT)

#define XPSR_DEFAULT                 SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS)
#else
/* For Cortex-M */
#define XPSR_DEFAULT                 0x01000000
#endif
/* For Riscv */
#define MSTATUS_DEFAULT              0x00001800
/*---------------------------------------------------------------------------*/
typedef struct {
  uintptr_t pc;
  union {
    uint32_t spsr; /* Arm */
    uint32_t mstatus; /* Riscv */
  };
} transfer_parameter;
/*---------------------------------------------------------------------------*/
void transfer_control(transfer_parameter *param);
/*---------------------------------------------------------------------------*/
void prepare_transfer_param(uintptr_t offset, transfer_parameter *param);
/*---------------------------------------------------------------------------*/

#endif
