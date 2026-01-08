#include <stdint.h>
#include "cinit.h"
#include "syscall-arom.h"
#include "log.h"
#include "bl2_arg.h"
#ifdef MODULE_LOADTABLE
#include "loadtable.h"
#endif
#include "pmu.h"

/*---------------------------------------------------------------------------*/
extern int main(void);
/*---------------------------------------------------------------------------*/
__attribute__((target("arm"), section(".text.boot")))
int
_start(bl2_arg_t *arg)
{
#ifdef MODULE_LOADTABLE
  if(process_loadtable() < 0) {
    return -1;
  }
#endif

  cinit_clear_bss();
  syscall_init();

  bl2_arg_init(arg);
  arg = bl2_arg_get();
  if(arg && arg->disable_log) {
    log_set_level("main", LOG_LEVEL_ERR);
  } else {
    if(usb_connect() == 0) {
      log_set_level("main", LOG_LEVEL_PRINT);
    }
  }

  cinit_call_constructors();

  int rc = main();

  return rc;
}
