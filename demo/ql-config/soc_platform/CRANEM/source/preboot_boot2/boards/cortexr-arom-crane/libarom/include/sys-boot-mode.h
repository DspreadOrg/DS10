/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */

#ifndef SYS_BOOT_MODE_H
#define SYS_BOOT_MODE_H

/**
 * BOOTMODE definition
 */
typedef enum {
  SYS_BOOT_MODE_NORMAL,
  SYS_BOOT_MODE_FORCE_DOWNLOAD,
  SYS_BOOT_MODE_TRY_DOWNLOAD,
  SYS_BOOT_MODE_PRODUCTION,
  SYS_BOOT_MODE_MAX
} sys_boot_mode_t;

#endif /* SYS_BOOT_MODE_H */
/** @} */
