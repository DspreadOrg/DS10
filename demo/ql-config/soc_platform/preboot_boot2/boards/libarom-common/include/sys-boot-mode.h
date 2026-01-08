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

/**
 * Get system boot mode
 *
 * \return current boot mode.
 */
sys_boot_mode_t sys_boot_mode_get(void);

/**
 * Set system boot mode
 *
 * \param boot_mode The new boot mode will be set
 */
void sys_boot_mode_set(sys_boot_mode_t boot_mode);

#endif /* SYS_BOOT_MODE_H */
/** @} */
