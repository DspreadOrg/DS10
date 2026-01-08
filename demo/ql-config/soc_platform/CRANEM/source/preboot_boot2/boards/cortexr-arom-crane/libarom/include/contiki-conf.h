#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

/*---------------------------------------------------------------------------*/
/* Include Project Specific conf */
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif /* PROJECT_CONF_PATH */
/*---------------------------------------------------------------------------*/
#include "crane-def.h"

#define PLATFORM_CONF_PROVIDES_MAIN_LOOP      1
#define QUEUEBUF_CONF_ENABLED                 0
#define GPIO_HAL_CONF_PORT_PIN_NUMBERING      0

#endif /* CONTIKI_CONF_H */
