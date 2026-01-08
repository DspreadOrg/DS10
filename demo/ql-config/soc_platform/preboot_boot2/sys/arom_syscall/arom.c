#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "syscall-arom.h"

/*---------------------------------------------------------------------------*/
/* arom.c AROM_VER_2020_07_30_CRANEGM_A0 */
/*---------------------------------------------------------------------------*/
static int
arom_production_mode_get_2020_07_30(void)
{
  /* arom_production_mode_get is not has in bootrom version: 2020.07.30 */
  return -1;
}
/*---------------------------------------------------------------------------*/
void
arom_syscall_register(void)
{
  /* arom.c */
  syscall_register(SYSCALL_91_PRODUCTION_MODE_GET,
                   (void *)arom_production_mode_get_2020_07_30);
}
/*---------------------------------------------------------------------------*/
