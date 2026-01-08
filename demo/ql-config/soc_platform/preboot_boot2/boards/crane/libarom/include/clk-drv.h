#ifndef _CLK_DRV_H_
#define _CLK_DRV_H_

#include "syscall-arom.h"

/*
 * QSPI clock freqency definitions
 */
 #if 0
/* cr5 z1 diff with other cranes */
#define QSPI_CLOCK_13MHZ_CR5_Z1    0
#define QSPI_CLOCK_26MHZ_CR5_Z1    1
#define QSPI_CLOCK_48MHZ_CR5_Z1    2
#define QSPI_CLOCK_52MHZ_CR5_Z1    3
#define QSPI_CLOCK_78MHZ_CR5_Z1    4
#define QSPI_CLOCK_104MHZ_CR5_Z1   5
#endif
/*              freq           0     1     2     3     4     5    6     7
 * Z1 new map:  clock[8]   = { 13,   13,   13,  26,   48,   52,   78,  104 };
 * Z2:          clock[8]   = { 13,   13,   13,  26,   39,   52,   78,  104 };
 */
#define QSPI_CLOCK_13MHZ    1
#define QSPI_CLOCK_26MHZ    3
#define QSPI_CLOCK_39MHZ    4
#define QSPI_CLOCK_52MHZ    5
#define QSPI_CLOCK_78MHZ    6
#define QSPI_CLOCK_104MHZ   7

#endif
