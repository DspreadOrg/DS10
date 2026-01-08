#ifndef __FLASH_H__
#define __FLASH_H__
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pt.h>
#include <process.h>

#ifndef FLASH_VERIFY_EN
#define FLASH_VERIFY_EN  (1)  /* defaultly need verify flash written data */
#endif

#ifndef FLASH_CHECK_FF_EN
#define FLASH_CHECK_FF_EN  (1)  /* defaultly need read flash and check whether are erased, all 0xFF */
#endif

#ifndef SLT_ENABLE
#define SLT_ENABLE      0       /* disable SLT test */
/* #define SLT_ENABLE   1 */    /* enable SLT test mode */
#endif

#if SLT_ENABLE == 1
/*
 * SLT test need, if reboot command choosen, replace pmic sw reset to boot to boot33.
 * boot33 binary is in external QSPI flash, A2
 * ptable saved in btcm 4K.
 */
/* suport add QSPI A2 external flash for SLT_ENABLE == 1 use 32MB flash */
#define FLASH_SUPPORT_SUBSTITUTE (1)
#endif

#define FLASH_INFO_MAGIC        0x46494E46
#define MAX_FLASH_DEVICE_NAME   64
#define MAX_FLASH_DEVICE_TYPE   32

#define FLASH_TYPE_QSPI_NAND    "QSPI_NAND"
/* the first QSPI flash, crane is extern QSPI flash, craneM if internal QSPI flash.*/
#define FLASH_TYPE_QSPI_NOR     "QSPI_NOR"
#define FLASH_TYPE_SPI_NOR      "SPI_NOR"
#define FLASH_TYPE_EMMC         "EMMC"
#define FLASH_TYPE_MEM          "MEM"
#define FLASH_TYPE_DUMMY        "DUMMY"

#ifdef FLASH_SUPPORT_SUBSTITUTE
/* the second QSPI flash, flash size(32MB)*/
#define FLASH_TYPE_QSPI2_NOR    "QSPI2_NOR"
#endif

#define FLASH_ERR_OTHERS        (-1)
#define FLASH_ERR_BAD_BLOCK     (-2)
#define FLASH_ERR_BAD_PARAMETER (-3)
#define FLASH_ERR_ECC_ERR       (-4)
#define FLASH_ERR_PROGRAM_ERR   (-5)
#define FLASH_ERR_ERASE_ERR     (-6)

#define SPI_PORT_QSPI           0
#define SPI_PORT_SSP0           1
#define SPI_PORT_SSP1           2
#define SPI_PORT_SSP2           3
#define SPI_PORT_QSPI2          4

struct compatible_device {
  uint16_t vendor_id;
  uint16_t device_id;
  uint32_t reserved1;
  char device_name[MAX_FLASH_DEVICE_NAME];
};

/**
 * \brief Flash information
 */
struct flash_info {
  uint32_t magic;
  uint16_t vendor_id;
  uint16_t device_id;
  char device_name[MAX_FLASH_DEVICE_NAME];
  char device_type[MAX_FLASH_DEVICE_TYPE];
  /* the following two parameters are used by ubi */
  uint32_t min_io_size;
  uint32_t peb_size;
  /* the following three parameters are used by erase and program */
  uint32_t page_size;
  uint32_t sector_size;
  uint32_t block_size;
  uint32_t reserved1;
  /* total flash size */
  uint64_t flash_size;
  uint8_t erased_value;       /*/< Contents of erased memory (usually 0xFF) */
  uint8_t chained;
  uint8_t spi_port;
  uint8_t combined_flash_size_in_mb; /* combined flash size in MB */
  uint32_t compatible_device_num;
  struct compatible_device compatible_device_list[1];
};

typedef struct {
  uint32_t pnum;
  uint32_t offset;
} ubi_addr_t;

typedef struct {
  const void *flash;
  size_t partition_size;
} sparse_param_t;

typedef struct {
  /* Containers for the State */
  struct pt pt;

  /* Input Variables */
  process_event_t ev;
  union {
    uint64_t addr;
    ubi_addr_t ubi_addr;
    sparse_param_t sparse_param;
  };
  const void *data;
  size_t size;

  /* Output Variables */
  int result;               /**< Result Code */
} flash_state_t;

typedef int (*flash_read_t)(uint64_t addr, void *data, size_t size);
typedef int (*flash_copy_t)(uint64_t src_addr, uint64_t dst_addr, size_t size);
typedef char (*flash_write_t)(flash_state_t *state);
typedef char (*flash_erase_t)(flash_state_t *state);
typedef const struct flash_info *(*flash_get_info_t)(void);

/*---------------------------------------------------------------------------*/
typedef void (*flash_quad_enable_cb_t)(bool enable);
/*---------------------------------------------------------------------------*/

struct flash_operation {
  flash_read_t read;
  flash_write_t write;
  flash_erase_t erase;
  flash_copy_t copy;
  flash_get_info_t info;
};

typedef struct {
  uint8_t id_external;
  uint8_t id_internal;
  uint8_t id_external2;
  uint8_t reserved;
} flash_id_en_config_t;

typedef enum {
  FLASH_ID_EXTERNAL,        /* external/internal qspi flash */
  FLASH_ID_INTERNAL,        /* internal spi flash */
  FLASH_ID_EXTERNAL2,       /* external2 qspi flash */
  FLASH_ID_MAX
} flash_id_t;

/* start a protothread to do thread_fun(uint64_t address, const void *buf, size_t sz).
 * state: supply parent pt for thread_fun.
 * child_state: pt for thread_fun.(if multi pt threads, child_state may be custom defined)
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is partition_raw_flash(uint64_t address, const void *buf, size_t sz),
 * invoke FLASH_BASE_TASK(state, partition_raw_flash_state, partition_raw_flash, address, buf, sz);
 */
#define FLASH_BASE_TASK(state, child_state, thread_fun, address, buf, sz) \
  { \
    static flash_state_t child_state; \
    memset(&child_state, 0, sizeof(child_state)); \
    child_state.ev = state->ev; \
    child_state.addr = address; \
    child_state.data = buf; \
    child_state.size = sz; \
    PT_SPAWN(&state->pt, &(child_state.pt), thread_fun(&child_state)); \
    state->result = child_state.result; \
  }

#define INPUT_VOID_TASK(state, child_state, thread_fun) \
  FLASH_BASE_TASK(state, child_state, thread_fun, 0, NULL, 0)

/* start a protothread to do thread_fun(void).
 * state: supply parent pt for thread_fun(void).
 * state->result: return result of thread_fun(void).
 *
 * eg: if thread_fun(void) is wait_for_complete_task(void),
 * invoke WAIT_TASK(state, wait_for_complete_task);
 */
#define WAIT_TASK(state, thread_fun) \
  FLASH_BASE_TASK(state, wait_state, thread_fun, 0, NULL, 0)

/* start a protothread to do thread_fun(uint64_t address, size_t sz).
 * state: supply parent pt for thread_fun.
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is flash->erase(uint64_t address, size_t sz),
 * invoke FLASH_ERASE_TASK(state, flash->erase, address, sz);
 */
#define FLASH_ERASE_TASK(state, thread_fun, address, sz) \
  FLASH_BASE_TASK(state, erase_state, thread_fun, address, NULL, sz)

/* start a protothread to do thread_fun(uint64_t address, const void *buf, size_t sz).
 * state: supply parent pt for thread_fun.
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is flash->write(uint64_t address, const void *buf, size_t sz),
 * invoke FLASH_WRITE_TASK(state, flash->write, address, buf, sz);
 */
#define FLASH_WRITE_TASK(state, thread_fun, address, buf, sz) \
  FLASH_BASE_TASK(state, write_state, thread_fun, address, buf, sz)

int flash_probe(flash_id_t flash_id);
int flash_info_init(const void *data, size_t size);
int flash_all_probe(void);
int flash_force_all_probe(void);
const struct flash_operation *flash_set_operation(flash_id_t flash_id);
const struct flash_operation *flash_get_operation(void);
int flash_verify(uint64_t addr, const void *data, size_t size);

bool flash_is_chip_erased(flash_id_t id);
void flash_set_chip_erased(flash_id_t id);
void flash_reset_chip_erased(void);
flash_id_t flash_get_current_flash_id(void);

#define FLASH_IS_ALL_FF_TASK(state, address, sz) \
  FLASH_BASE_TASK(state, flash_is_all_ff_state, flash_is_all_ff, address, NULL, sz)

PT_THREAD(flash_is_all_ff(flash_state_t * state));

#ifdef FLASH_SUPPORT_SUBSTITUTE
int flash_force_probe_substitute(void);
#endif

#endif /* __FLASH_H__ */
