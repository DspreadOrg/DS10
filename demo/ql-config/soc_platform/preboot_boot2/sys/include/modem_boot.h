/**
 * @file
 * @author  Jinhua Huang <jinhuahuang@asrmicro.com>
 * @version 1.0
 *
 * @section LICENSE
 * Copyright (C) 2021, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 * Aboot tiny downloader for Jacana with firmware and pvt
 *
 */
#ifndef __MODEM_BOOT_H__
#define __MODEM_BOOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

int modem_boot_init(void);
int modem_boot_load(void *handle);
void modem_boot_exit(void);
int modem_boot_run(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __MODEM_BOOT_H__ */
