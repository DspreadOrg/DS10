#ifndef CONTIKI_CONF_H
#define CONTIKI_CONF_H

/*---------------------------------------------------------------------------*/
/* Include Project Specific conf */
#ifdef PROJECT_CONF_PATH
#include PROJECT_CONF_PATH
#endif /* PROJECT_CONF_PATH */
/*---------------------------------------------------------------------------*/
#if defined(CPU_MODEL_CRANE) || defined(CPU_MODEL_CRANEW_MCU)
#include "crane-def.h"
#endif

#define PLATFORM_CONF_PROVIDES_MAIN_LOOP      1
#define QUEUEBUF_CONF_ENABLED                 0
#define GPIO_HAL_CONF_PORT_PIN_NUMBERING      0

#endif /* CONTIKI_CONF_H */
