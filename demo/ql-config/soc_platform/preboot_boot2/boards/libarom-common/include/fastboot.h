/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
#ifndef FASTBOOT_H
#define FASTBOOT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "syscall-arom.h"

#define MAX_RSP_SIZE            128
#define MAX_GET_VAR_NAME_SIZE   64

unsigned hex2unsigned(const char *x);

int fastboot_init(void);
void fastboot_stop(void);

/* register a command handler
 * - command handlers will be called if their prefix matches
 * - they are expected to call fastboot_okay() or fastboot_fail()
 *   to indicate success/failure before returning
 */
void fastboot_register(const char *prefix,
                       void (*handle)(const char *arg, void *data, unsigned size));

/* publish a variable readable by the built-in getvar command */
void fastboot_publish(const char *name, const char *value);

/* cleanup partition information variable */
void fastboot_unpublish_partition(void);

/*
 * fastboot report functions
 *
 * only callable from within a command handler
 */
void fastboot_okay(const char *result);
void fastboot_fail(const char *reason);

/*char *fastboot_format(const char *fmt, ...); */
typedef char *(*fastboot_format_t)(const char *fmt, ...);
#define fastboot_format(...) ((fastboot_format_t)syscall_get_handler(SYSCALL_54_FASTBOOT_FORMAT))(__VA_ARGS__)

void fastboot_info(const char *reason);
void fastboot_progress(unsigned progress);

/*
 * fastboot command handling
 */
/* execute command loop once */
void fastboot_command_loop_once(const uint8_t *data, size_t size);
/* check if cmd is download command */
bool fastboot_is_download_cmd(const uint8_t *data, size_t size);
/* check if cmd is flasher command */
bool fastboot_is_flasher_cmd(const uint8_t *data, size_t size);

/*
 * fastboot set / get reboot flag
 */
void fastboot_reboot_set(bool reboot);
bool fastboot_reboot_get(void);

/*
 * fastboot set / get boot flag
 */
void fastboot_boot_set(bool boot);
bool fastboot_boot_get(void);

/*
 * fastboot set / get go flag
 */
void fastboot_go_set(bool go);
bool fastboot_go_get(void);

/*
 * fastboot set / get disconnect flag
 */
void fastboot_disconnect_set(bool disconnect);
bool fastboot_disconnect_get(void);

void fastboot_set_state_complete(void);

/* download block */
int fastboot_block_init(void *xfer_buffer, unsigned max, unsigned division);
void *fastboot_block_alloc(void);
void fastboot_block_free(void *);
int fastboot_block_numfree(void);

/* download/upload data handling */
void fastboot_download_data(const uint8_t *data, size_t size);
bool fastboot_download_is_verified(void);
void fastboot_download_set_verified(void);
void fastboot_upload_area_set(void *base, unsigned size);
void fastboot_upload_data(const uint8_t *data, size_t size);

/* step progress */
void fastboot_progress_step(size_t step);

uint8_t fastboot_state_get(void);
void fastboot_state_set(uint8_t state);

/* only for AROM_VER_2020_07_30_CRANEGM_A0 */
bool is_fastboot_all_completed_2020_07_30(void);

#endif /* FASTBOOT_H */
