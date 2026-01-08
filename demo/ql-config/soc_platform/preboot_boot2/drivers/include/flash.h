#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

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

#define FLASH_INFO_MAGIC        0x46494E46
#define MAX_FLASH_DEVICE_NAME   64
#define MAX_FLASH_DEVICE_TYPE   32

#define FLASH_TYPE_QSPI_NAND    "QSPI_NAND"
/* the first QSPI flash, crane is extern QSPI flash, craneM/craneG is internal QSPI flash.*/
#define FLASH_TYPE_QSPI_NOR     "QSPI_NOR"
#define FLASH_TYPE_SPI_NOR      "SPI_NOR"
#define FLASH_TYPE_SPI_NAND     "SPI_NAND"
#define FLASH_TYPE_EMMC         "EMMC"
#define FLASH_TYPE_MEM          "MEM"
#define FLASH_TYPE_DUMMY        "DUMMY"
#define FLASH_TYPE_QSPI2_NOR    "QSPI2_NOR"

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
#define SPI_PORT_EMMC           5
#define SPI_PORT_MEM            6

/* SSP_<CLK>_<FRM>_<RXD>_<TXD> */
typedef enum {
  SSP_GPIO_CONFIG_DEFAULT,
  SSP_GPIO4_FUNC2_GPIO5_FUNC2_GPIO6_FUNC2_GPIO7_FUNC2,
  SSP_GPIO4_FUNC7_GPIO5_FUNC7_GPIO6_FUNC7_GPIO7_FUNC7,
  SSP_GPIO12_FUNC1_GPIO13_FUNC1_GPIO14_FUNC1_GPIO15_FUNC1,
  SSP_GPIO16_FUNC2_GPIO17_FUNC2_GPIO19_FUNC2_GPIO18_FUNC2,
  SSP_GPIO16_FUNC2_GPIO17_FUNC2_GPIO18_FUNC4_GPIO19_FUNC4,
  SSP_GPIO33_FUNC1_GPIO34_FUNC1_GPIO35_FUNC1_GPIO36_FUNC1,
  SSP_GPIO33_FUNC2_GPIO34_FUNC2_GPIO35_FUNC2_GPIO36_FUNC2,
  SSP_GPIO64_FUNC7_GPIO63_FUNC7_GPIO61_FUNC7_GPIO62_FUNC7,
  SSP_GPIO71_FUNC2_GPIO72_FUNC2_GPIO73_FUNC2_GPIO74_FUNC2,
  SSP_QSPI_CLK_FUNC2_QSPI_CS1_FUNC2_QSPI_DAT1_FUNC2_QSPI_DAT0_FUNC2,
  SSP_QSPI_CLK_FUNC4_QSPI_CS1_FUNC4_QSPI_DAT1_FUNC4_QSPI_DAT0_FUNC4,
  SSP_GPIO4_FUNC2_GPIO55_FUNC0_GPIO6_FUNC2_GPIO7_FUNC2,
} ssp_gpio_config_t;

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
  uint8_t reserved1[3];
  uint8_t ssp_gpio : 6;
  uint8_t ssp_clk  : 2;
  /* total flash size */
  uint64_t flash_size;
  uint8_t erased_value;       /*/< Contents of erased memory (usually 0xFF) */
  uint8_t chained;
  uint8_t spi_port;
  uint8_t combined_flash_size_in_mb; /* combined flash size in MB */
  uint32_t compatible_device_num;
  struct compatible_device compatible_device_list[1];
};

typedef enum {
  SSP_CLK_52M,
  SSP_CLK_26M,
  SSP_CLK_13M,
  SSP_CLK_6500K,
} ssp_clk_config_t;

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
  size_t repeats;

  /* Output Variables */
  int result;               /**< Result Code */
} flash_state_t;

/* support spi_nor_flash.c use qspi controller common interface, although spi did not use qspi controller */
typedef int (*qspi_read_t)(unsigned seqid, unsigned addr, void *data, size_t size);
typedef int (*qspi_write_t)(unsigned seqid, unsigned addr, const void *data, size_t size);
typedef void (*qspi_cmd_t)(unsigned seqid, unsigned addr);

typedef struct {
  qspi_read_t qspi_read;
  qspi_write_t qspi_write;
  qspi_cmd_t qspi_cmd;
} qpsi_lut_operation_t;

typedef int (*flash_read_t)(uint64_t addr, void *data, size_t size);
typedef char (*flash_write_t)(flash_state_t *state);
typedef char (*flash_erase_t)(flash_state_t *state);
typedef const struct flash_info *(*flash_get_info_t)(void);

/*---------------------------------------------------------------------------*/
typedef void (*flash_quad_enable_cb_t)(const qpsi_lut_operation_t *lut_op, bool enable);
/*---------------------------------------------------------------------------*/

struct flash_operation {
  flash_read_t read;
  flash_write_t write;
  flash_erase_t erase;
  flash_get_info_t info;
};

typedef enum {
  FLASH_ID_FIRST,         /* first flash */
  FLASH_ID_SECOND,        /* second flash */
  FLASH_ID_THIRD,         /* third flash */
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
#define FLASH_BASE_TASK(state, child_state, thread_fun, address, buf, sz, repts) \
  { \
    static flash_state_t child_state; \
    memset(&child_state, 0, sizeof(child_state)); \
    child_state.ev = state->ev; \
    child_state.addr = address; \
    child_state.data = buf; \
    child_state.size = sz; \
    child_state.repeats = repts; \
    PT_SPAWN(&state->pt, &(child_state.pt), thread_fun(&child_state)); \
    state->result = child_state.result; \
  }

#define INPUT_VOID_TASK(state, child_state, thread_fun) \
  FLASH_BASE_TASK(state, child_state, thread_fun, 0, NULL, 0, 0)

/* start a protothread to do thread_fun(void).
 * state: supply parent pt for thread_fun(void).
 * state->result: return result of thread_fun(void).
 *
 * eg: if thread_fun(void) is wait_for_complete_task(void),
 * invoke WAIT_TASK(state, wait_for_complete_task);
 */
#define WAIT_TASK(state, thread_fun) \
  FLASH_BASE_TASK(state, wait_state, thread_fun, 0, NULL, 0, 0)

/* start a protothread to do thread_fun(uint64_t address, size_t sz).
 * state: supply parent pt for thread_fun.
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is flash->erase(uint64_t address, size_t sz),
 * invoke FLASH_ERASE_TASK(state, flash->erase, address, sz);
 */
#define FLASH_ERASE_TASK(state, thread_fun, address, sz) \
  FLASH_BASE_TASK(state, erase_state, thread_fun, address, NULL, sz, 0)

/* start a protothread to do thread_fun(uint64_t address, const void *buf, size_t sz, size_t repeats).
 * state: supply parent pt for thread_fun.
 * state->result: return result of thread_fun.
 *
 * eg: if thread_fun is flash->write(uint64_t address, const void *buf, size_t sz, size_t repeats),
 * invoke FLASH_WRITE_TASK(state, flash->write, address, buf, sz, repeats);
 */
#define FLASH_WRITE_TASK(state, thread_fun, address, buf, sz, repeats) \
  FLASH_BASE_TASK(state, write_state, thread_fun, address, buf, sz, repeats)

int flash_info_init(const void *data, size_t size);
int flash_force_all_probe(void);
const struct flash_operation *flash_set_operation(flash_id_t flash_id);
const struct flash_operation *flash_get_operation(void);
const struct flash_operation *flash_get_operation_with_id(flash_id_t flash_id);
int flash_verify(uint64_t addr, const void *data, size_t size);

bool flash_is_chip_erased(flash_id_t id);
void flash_set_chip_erased(flash_id_t id);
void flash_reset_chip_erased(void);
flash_id_t flash_get_current_flash_id(void);

/* read current flash from any address */
int flash_read_unaligned(uint64_t addr, void *data, size_t size);

uint8_t *flash_get_tmp_buffer(size_t sz);

#define FLASH_IS_ALL_FF_TASK(state, address, sz) \
  FLASH_BASE_TASK(state, flash_is_all_ff_state, flash_is_all_ff, address, NULL, sz, 0)

PT_THREAD(flash_is_all_ff(flash_state_t * state));

#ifdef USE_QSPI_NOR_FLASH
extern bool flash_with_qspi2_nor;
int flash_nor_write_by_page(uint64_t address, const void *data, size_t size);
int flash_nor_erase_by_sector(uint64_t address, size_t size);
#endif

#ifdef USE_QSPI_NAND_FLASH
int flash_nand_write_by_page(uint64_t address, const void *data, size_t size);
int flash_nand_erase_by_sector(uint64_t address, size_t size);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __FLASH_H__ */
