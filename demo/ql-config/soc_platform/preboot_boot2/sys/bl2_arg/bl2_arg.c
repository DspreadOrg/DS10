#include <stdio.h>
#include <string.h>
#include "bl2_arg.h"

static bl2_arg_t garg;

/*---------------------------------------------------------------------------*/
#ifdef __ARM_ARCH_ISA_ARM
__attribute__((always_inline)) static inline
uint32_t
__get_SP(void)
{
  uint32_t result;
  __asm__ volatile ("MOV %0, sp" : "=r" (result) : : "memory");
  return result;
}
#else
__attribute__((always_inline)) static inline
uint32_t
__get_MSP(void)
{
  uint32_t result;
  __asm__ volatile ("MRS %0, msp" : "=r" (result));
  return result;
}
#endif
/*---------------------------------------------------------------------------*/
static uint32_t
get_sp(void)
{
  uint32_t sp;

#ifdef __ARM_ARCH_ISA_ARM
  sp = __get_SP();
#else
  sp = __get_MSP();
#endif

  return sp;
}
/*---------------------------------------------------------------------------*/
bl2_arg_t *
bl2_arg_get(void)
{
  bl2_arg_t *p = &garg;
  if(p->magic == BL2_ARG_MAGIC) {
    return p;
  } else {
    return NULL;
  }
}
/*---------------------------------------------------------------------------*/
int
bl2_arg_init(bl2_arg_t *arg)
{
  uint32_t start, end, arg_addr;
  bl2_arg_t *p = &garg;

  start = get_sp();
  end = start + BL2_ARG_DETECT_SIZE;
  arg_addr = (uint32_t)arg;

  if((arg_addr > start) && (arg_addr <= end)) {
    if(arg->magic == BL2_ARG_MAGIC) {
      memcpy(p, arg, sizeof(bl2_arg_t));
      return 0;
    }
  }
  memset(p, 0x0, sizeof(bl2_arg_t));
  return -1;
}
/*---------------------------------------------------------------------------*/
