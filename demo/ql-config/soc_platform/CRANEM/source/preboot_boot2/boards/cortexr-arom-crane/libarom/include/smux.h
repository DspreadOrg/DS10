#if 0
#ifndef SMUX_H_
#define SMUX_H_

#include <stdint.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
/**
 * SMUX low-level device definition
 */
typedef enum {
  SMUX_DEV_UART,
  SMUX_DEV_USB,
  SMUX_DEV_MAX
} smux_dev_t;

/**
 * SMUX preamble type definition
 */
#define SMUX_PREAMBLE_SIZE  4       /* preamble char numbers */
#define SMUX_PREAMBLE_UUUU  "UUUU"  /* aboot standard */
#define SMUX_PREAMBLE_UABT  "UABT"  /* aboot tiny */
#define SMUX_PREAMBLE_PROD  "PROD"  /* aboot production mode */

/*---------------------------------------------------------------------------*/
/**
 * SMUX running mode definition
 */
typedef enum {
  SMUX_MODE_BYPASS,
  SMUX_MODE_PREAMBLE,
  SMUX_MODE_NORMAL,
  SMUX_MODE_MAX
} smux_mode_t;


typedef void (*smux_preamble_callback_t)(const char *preamble, smux_dev_t dev);
void smux_register_preamble_callback(smux_preamble_callback_t cb);
smux_dev_t smux_get_device(void);
void smux_use_device(smux_dev_t dev);
void smux_write_stdout(const uint8_t *data, size_t len);
smux_mode_t smux_running_mode_get(void);
void smux_running_mode_set(smux_mode_t mode);

#endif /* SMUX_H_ */

#endif

/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */

#ifndef SMUX_H
#define SMUX_H

#include "contiki.h"

/*---------------------------------------------------------------------------*/
/**
 * \brief The SMUX memory block config
 */
#ifndef SMUX_MEMB_NUM
#define SMUX_MEMB_NUM 8
#endif /* SMUX_MEMB_NUM */

#ifndef SMUX_MEMB_SIZE
#define SMUX_MEMB_SIZE 512
#endif /* SMUX_MEMB_SIZE */

/*---------------------------------------------------------------------------*/
/**
 * \brief The SMUX frame mtu size config
 */
#ifndef SMUX_FRAME_MTU
#define SMUX_FRAME_MTU 1024
#endif /* SMUX_FRAME_MTU */

/*---------------------------------------------------------------------------*/
/**
 * SMUX low-level device definition
 */
typedef enum {
  SMUX_DEV_UART,
  SMUX_DEV_USB,
  SMUX_DEV_MAX
} smux_dev_t;

/*---------------------------------------------------------------------------*/
/**
 * SMUX running mode definition
 */
typedef enum {
  SMUX_MODE_BYPASS,
  SMUX_MODE_PREAMBLE,
  SMUX_MODE_NORMAL,
  SMUX_MODE_MAX
} smux_mode_t;

/*---------------------------------------------------------------------------*/
/**
 * Input SMUX data bytes.
 *
 * This function is called by the RS232/SIO/UART/ACM device driver to pass
 * incoming bytes to the SMUX driver. The function can be called from
 * an interrupt context.
 *
 * For systems using low-power CPU modes, the return value of the
 * function can be used to determine if the CPU should be woken up or
 * not. If the function returns non-zero, the CPU should be powered
 * up. If the function returns zero, the CPU can continue to be
 * powered down.
 *
 * \param dev Which device data from
 * \param data The data start address that is to be passed to the SMUX driver
 * \param len The length of bytes that is to be passed to the SMUX driver
 *
 * \return Non-zero if the CPU should be powered up, zero otherwise.
 */
int smux_input_bytes(smux_dev_t dev, uint8_t *data, int len);

/**
 * Send using SMUX len bytes starting from the location pointed to by data
 */
void smux_write_stdout(const uint8_t *data, size_t len);
void smux_write_aboot_resp(const uint8_t *data, size_t len);
void smux_write_aboot_data(const uint8_t *data, size_t len);
void smux_write_heart_beat(void);

/*
 * Flush write frame queue to memory block
 *
 */
void smux_flush_write_queue(void);

/*
 * These machine dependent functions and an interrupt service routine
 * must be provided externally (smux_arch.c).
 */
void smux_arch_uart_init(void);
void smux_arch_uart_sync_baud_rate(void);
void smux_arch_uart_tx_start(void);
void smux_arch_uart_rx_start(void);
void smux_arch_uart_exit(void);
void smux_arch_usb_init(void);
void smux_arch_usb_tx_start(void);
void smux_arch_usb_rx_start(void);
void smux_arch_usb_exit(void);

/**
 * SMUX preamble type definition
 */
#define SMUX_PREAMBLE_SIZE  4       /* preamble char numbers */
#define SMUX_PREAMBLE_UUUU  "UUUU"  /* aboot standard */
#define SMUX_PREAMBLE_UABT  "UABT"  /* aboot tiny */
#define SMUX_PREAMBLE_PROD  "PROD"  /* aboot production mode */
/*
 * Preamble handling
 */
typedef void (*smux_preamble_callback_t)(const char *preamble, smux_dev_t dev);
void smux_register_preamble_callback(smux_preamble_callback_t cb);
void smux_use_device(smux_dev_t dev);
smux_dev_t smux_get_device(void);

/**
 * SMUX aboot cmd & data handling
 */
typedef void (*smux_aboot_callback_t)(const uint8_t *data, size_t len);
void smux_register_aboot_cmd_callback(smux_aboot_callback_t cb);
void smux_register_aboot_data_callback(smux_aboot_callback_t cb);
void smux_register_aboot_upload_callback(smux_aboot_callback_t cb);

/**
 * SMUX memory block alloc and free
 */
void *smux_memb_alloc(void);
void smux_memb_free(void *ptr);

/*
 * Get an out data packet from the sending queue
 *
 * This function is called by the RS232/SIO/UART/ACM device driver to get
 * a new outgoing packet to be sent by low-level device driver.
 *
 * The low-level device driver should free the returned data after data
 * has been sent with function `smux_memb_free`
 *
 * \param size The size address to hold the actually size of the data
 *
 * \return Non-NULL if data available otherwise NULL.
 *
 */
void *smux_get_out_data(size_t *size);

/**
 * SMUX module initialization
 */
void smux_init(void);

/**
 * Close working SMUX arch device
 */
void smux_exit(void);

/**
 * SMUX running mode operation
 */
smux_mode_t smux_running_mode_get(void);
void smux_running_mode_set(smux_mode_t mode);

#endif /* SMUX_H */
/** @} */

