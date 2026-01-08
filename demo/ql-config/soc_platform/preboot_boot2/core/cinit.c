#include <stdint.h>
#include "cinit.h"

extern void *__ctor_list, *__ctor_end;
extern int __bss_start, __bss_end;

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
