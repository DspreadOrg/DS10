/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */

#ifndef SECUREBOOT_H
#define SECUREBOOT_H

#include <stddef.h>
#include <stdint.h>

enum {
    VOLUME_TYPE_UBI,
    /* add new type here */
    VOLUME_TYPE_MAX,
};
struct volume_info_t {
    int type;
    void *info;
};

/*
 * volume access for bootloader partition
 */
int sb_volume_init(void);
int sb_volume_read(const char *name, void *base, size_t size);
void sb_volume_exit(void);

/*
 * Setup bl2 image runtime address spaces
 */
int sb_bl2_image_setup(void *base, size_t size);

/*
 * Setup FIP image load address spaces
 */
int sb_fip_image_setup(void *base, size_t size);

/*
 * Auth sub-image from FIP in memory and load to target address
 */
int sb_auth_image(uint32_t image_id, void *fip_start, void *load_target);

/*
 * Boot into bl2
 */
void sb_boot_bl2(void);

/*
 * Call image with image_id
 */
int sb_call_image(uint32_t image_id);

/*
 *  Read data from boot flash device
 */
int sb_flash_read(uint64_t addr, void *data, size_t size);

#endif /* SECUREBOOT_H */
/** @} */
