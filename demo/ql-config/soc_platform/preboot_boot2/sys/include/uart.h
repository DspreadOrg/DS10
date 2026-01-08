#ifndef __UART_H
#define __UART_H

#include <stdint.h>

/*---------------------------------------------------------------------------*/
void uart_init(void);
void uart_uninit(void);
void uart_out_sync(const char *str, int len);

/*---------------------------------------------------------------------------*/

#endif
