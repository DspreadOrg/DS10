#include <stdint.h>
#include "cinit.h"
#include "syscall-arom.h"

extern int main(void);

__attribute__((target("arm"), section(".text.boot")))
int _start(void)
{
    cinit_copy_patch();
    cinit_clear_bss();
    cinit_call_constructors();

    if(AROM_VER_2020_07_30 != arom_getversion()) {
      syscall_init();
    }
    int rc = main();

    return rc;
}
