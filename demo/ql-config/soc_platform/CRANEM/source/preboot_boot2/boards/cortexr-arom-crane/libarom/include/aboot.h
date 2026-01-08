/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
#ifndef ABOOT_H_
#define ABOOT_H_

#include "contiki.h"
#include "ptable.h"
#include "aboot_cmd.h"

/*---------------------------------------------------------------------------*/
/*
 * Flasher process interface
 */
typedef enum {
  FLASHER_CMD_ERASE,
  FLASHER_CMD_PROGRAM,
  FLASHER_CMD_FUSE,
  FLASHER_CMD_COMPLETE,
  FLASHER_CMD_MAX
} flasher_cmd_t;

typedef struct {
  flasher_cmd_t cmd;
  char partition[MAX_PTABLE_ENTRY_NAME];
  void *data;
  size_t size;
} flasher_message_t;

/*
 * Alloc a new message block
 */
flasher_message_t *aboot_flasher_message_new(void);

/*
 * Send flasher message from aboot to flasher
 */
int aboot_flasher_message_enqueue(flasher_message_t *msg);

/*
 * Get a message from aboot called by flasher
 */
flasher_message_t *aboot_flasher_message_dequeue(void);

/*
 * Called after flasher message process finished
 */
void aboot_flasher_message_release(flasher_message_t *msg);

/*---------------------------------------------------------------------------*/

/*
 * Init and start aboot module
 */
int aboot_start(void);

/*
 * Stop aboot module
 */
void aboot_stop(void);

/*
 * Queue aboot reboot command
 */
void aboot_reboot(void);
void aboot_boot(void);
void aboot_go(void);

/*
 * Register flasher process to aboot
 */
void aboot_register_flasher_process(struct process *process);

/*
 * Register reboot command callback
 */
typedef void (*reboot_cmd_t)(void);
void aboot_register_reboot_callback(reboot_cmd_t cb);
void aboot_register_go_callback(reboot_cmd_t cb);

#endif /* ABOOT_H_ */
