/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
#ifndef ABOOT_CMD_H
#define ABOOT_CMD_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* aboot command function pointer */
typedef void (*aboot_cmd_fn)(const char *, void *, unsigned);

typedef struct {
  char *name;
  aboot_cmd_fn cb;
} aboot_cmd_desc_t;

typedef void *(*upload_stage_cb_t)(void *data, unsigned size);
void aboot_register_upload_stage_cb(upload_stage_cb_t cb);
void aboot_download_scratch_setup(void *base, size_t size, unsigned division);
void aboot_register_ptable_info(void);
void register_commands(const aboot_cmd_desc_t *cmd_list);

void cmd_boot(const char *arg, void *data, unsigned sz);
void cmd_call(const char *arg, void *data, unsigned sz);
void cmd_erase(const char *arg, void *data, unsigned sz);
void cmd_flash(const char *arg, void *data, unsigned sz);
void cmd_fuse(const char *arg, void *data, unsigned sz);
void cmd_complete(const char *arg, void *data, unsigned sz);
void cmd_partition(const char *arg, void *data, unsigned sz);
void cmd_upload_stage(const char *arg, void *data, unsigned sz);
void cmd_useflash(const char *arg, void *data, unsigned sz);
void cmd_verify(const char *arg, void *data, unsigned sz);
void cmd_reboot(const char *arg, void *data, unsigned sz);

void aboot_command_init(const aboot_cmd_desc_t *cmd_list);
void aboot_command_exit(void);

const char *aboot_sys_getversion(void);

#endif /* ABOOT_CMD_H */
