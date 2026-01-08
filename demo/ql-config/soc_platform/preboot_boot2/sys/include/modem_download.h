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
#ifndef __MODEM_DOWNLOAD_H__
#define __MODEM_DOWNLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Aboot tiny init for modem
 *
 * @param name modem firmware path name
 * @param weight total progress weight for modem download, only valid on contiki
 *
 * @return 0 for success, otherwise failed
 */
int modem_aboot_tiny_init(const char *name, size_t weight);

/**
 * Aboot tiny cleanup for modem
 */
void modem_aboot_tiny_exit(void);

/**
 * Aboot tiny main loop
 *
 * @return 0 for success, > 0 for continue, < 0 failed
 */
int modem_aboot_tiny_run(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __MODEM_DOWNLOAD_H__ */
