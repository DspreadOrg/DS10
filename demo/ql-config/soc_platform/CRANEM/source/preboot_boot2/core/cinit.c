#include <stdint.h>
#include "cinit.h"
#include "cpu.h"

extern void *__ctor_list, *__ctor_end;
extern int __bss_start, __bss_end;
extern int __patch_load_addr, __patch_start, __patch_size;

/*---------------------------------------------------------------------------*/
void
cinit_clear_bss(void)
{
  int *p = &__bss_start;

  for(; p < &__bss_end; p++) {
    *p = 0;
  }
}
/*---------------------------------------------------------------------------*/
void
cinit_call_constructors(void)
{
  void **ctor;

  ctor = &__ctor_list;
  while(ctor != &__ctor_end) {
    void (*func)(void);

    func = (void (*)(void))(uintptr_t) * ctor;
    func();
    ctor++;
  }
}
/*---------------------------------------------------------------------------*/
void
cinit_copy_patch(void)
{
  unsigned int chip_id = hw_chip_id();

  /* arom_patch -> bl_encode also need judge chip id. */
  if(CHIP_ID_CRANEG != chip_id && CHIP_ID_CRANEM != chip_id) {
    return;
  }

  unsigned *src = (unsigned *)&__patch_load_addr;
  unsigned *dst = (unsigned *)&__patch_start;
  unsigned size = (unsigned)&__patch_size;

  while(size) {
    *dst++ = *src++;
    size -= 4;
  }
}
/*---------------------------------------------------------------------------*/
