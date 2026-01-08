#ifndef _VCOM_SERIAL_
#define _VCOM_SERIAL_

#include <stdint.h>
#include "cpu.h"
#include "shmrb.h"
#include "aboot-tiny.h"

typedef enum {
  VCOM_DEV_0,
  VCOM_DEV_MAX,
} vcom_dev_num_t;
/*---------------------------------------------------------------------------*/
typedef struct {
  unsigned int_val;              /* number of triggers sent to the opposite party */
  unsigned handle_int_val;       /* number of triggers processed by the opposite party */
  unsigned ack_int_val;          /* number of triggers sent by the opposite party*/
  unsigned handle_ack_int_val;   /* number of triggers processed by your party */
}trigger_t;
/*---------------------------------------------------------------------------*/
typedef struct {
#ifdef VCOM_MASTER
  shmrb_t rx_buf;
  trigger_t rx_trigger;
  shmrb_t tx_buf;
  trigger_t tx_trigger;
#else
  shmrb_t tx_buf;
  trigger_t tx_trigger;
  shmrb_t rx_buf;
  trigger_t rx_trigger;
#endif
} vcom_shmem_t;
/*---------------------------------------------------------------------------*/
int vcom_init(vcom_dev_num_t dev, aboot_tiny_uart_rx_callback_t rx_cb, void *arg);
void vcom_exit(vcom_dev_num_t dev);
int vcom_send(vcom_dev_num_t dev, const uint8_t *data, size_t size);
void smux_arch_vcom_init(void);
void smux_arch_vcom_tx_start(void);
void smux_arch_vcom_rx_start(void);
void smux_arch_vcom_exit(void);
/*---------------------------------------------------------------------------*/

#endif
