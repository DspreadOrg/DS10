#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <byteorder.h>
#include "syscall-arom.h"
#include "reg.h"
#include "secureboot.h"
#include "pmic.h"
#include "sys-boot-mode.h"
#include "sys_close_all.h"
#include "smux.h"
#include "etimer.h"
#include "clock.h"
#include "arom-crane.h"
#include "pmu.h"
#include "property.h"
#include "transfer-control.h"
#include "loader.h"
#include "ptable.h"
#include "cpu.h"
#include "flash.h"
#include "boot_img.h"
#include "heapmem.h"
#include "fastboot.h"
#include "tbbr_img_def.h"
#include "fip.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE "Boot2"
#define LOG_LEVEL  LOG_LEVEL_MAIN
/*---------------------------------------------------------------------------*/
#define QUEC_CUST_FEATURE 1
#if (defined(QUEC_CUST_FEATURE)&& (AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5))
#define DOWNLOAD_CMD_DETECT_WAIT_MSECONDS 1200 /* ms */
#else
#define DOWNLOAD_CMD_DETECT_WAIT_MSECONDS 1000 /* ms */
#endif
#define AROM_VERSION_CRANEW_SERIES        (AROM_VERSION == AROM_VER_2022_07_28_CRANEW_Z2)
#define AROM_VERSION_CRANE_SERIES         (AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0 || \
                                           AROM_VERSION == AROM_VER_2021_10_16_CRANEL_A0 || \
                                           AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5 || \
                                           AROM_VERSION == AROM_VER_2024_04_30_LAPWING_CR5 || \
                                           AROM_VERSION == AROM_VER_2022_11_11_CRANEG_CR5)
/*---------------------------------------------------------------------------*/
/**
 * Preamble message type
 */
enum {
  PREAMBLE_DOWNLOAD,
  PREAMBLE_PRODUCTION
};
/*---------------------------------------------------------------------------*/
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
static void crane_boot2_init(void);
static void boot2_preamble_callback(const char *preamble, smux_dev_t dev);
static void transfer_arom_do_download(void);
static int do_boot_pz1(void);
#if (defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU))
static char *boardid_string;
#endif
#endif

/*---------------------------------------------------------------------------*/
extern unsigned char __lzma_decode_start, __lzma_decode_size;
/*---------------------------------------------------------------------------*/
#if (defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU))
#define PROP_NAME_BOARDID     "bl2.boardid"
#endif
/*---------------------------------------------------------------------------*/
__attribute__((constructor)) static void
platform_init(void)
{
#if defined(CPU_MODEL_CRANE) || defined(CPU_MODEL_CRANEW_MCU)
#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
  sys_boot_mode_set(SYS_BOOT_MODE_NORMAL);
#else
  char r_cmd;

  pmic_init();

  pmic_get_restart_cmd(&r_cmd);
  if(r_cmd == RESTART_CMD_USB_DOWNLOAD) {
    sys_boot_mode_set(SYS_BOOT_MODE_FORCE_DOWNLOAD);
    pmic_set_restart_cmd(RESTART_CMD_NORMAL_POWERON);
  } else if(r_cmd == RESTART_CMD_FORCE_NORMAL_POWERON) {
    sys_boot_mode_set(SYS_BOOT_MODE_NORMAL);
    pmic_set_restart_status(RESTART_STATUS_NORMAL_POWERON);
  } else if(r_cmd == RESTART_CMD_PRODUCTION) {
    sys_boot_mode_set(SYS_BOOT_MODE_PRODUCTION);
    pmic_set_restart_status(RESTART_STATUS_PRODUCTION);
  } else {
    /* RESTART_CMD_NORMAL_POWERON and unknown cmd */
    sys_boot_mode_set(SYS_BOOT_MODE_TRY_DOWNLOAD);
    /* set_restart_status or set_restart_cmd will after cmd detect finished. */
  }
#endif
#endif
}
/*---------------------------------------------------------------------------*/
#if 0
static void
display_binary(void *data, size_t len)
{
#define BYTE_COUNT_EACH_LINE 8

  static const char hex[] = "0123456789ABCDEF";
  static const char seperator = ',';
  unsigned char *p = (unsigned char *)data;
  char stash[BYTE_COUNT_EACH_LINE * (2 + 2 + 1) + 1];
  size_t j = 0;
  for(size_t i = 0; i < len; i++) {
    stash[j++] = '0';
    stash[j++] = 'x';
    stash[j++] = hex[(p[i] >> 4) & 0xF];
    stash[j++] = hex[p[i] & 0xF];
    stash[j++] = seperator;
    if(j == sizeof(stash) - 1) {
      stash[j] = '\0';
      LOG_INFO("%s\n", stash);
      j = 0;
    }
  }
  if(j) {
    stash[j] = '\0';
    LOG_INFO("%s\n", stash);
  }
}
#endif
/*---------------------------------------------------------------------------*/
#define USB2_REG0B_DEFAULT_VAL 0x3000
/*---------------------------------------------------------------------------*/
static void
usb_check_reg_before_init(void)
{
  uint32_t reg_usb2_0A, reg_usb2_0B;
  uint32_t reg_usb2_0A_val, reg_usb2_0B_val;

  reg_usb2_0A = UTMI_BASE + UTMI_USB2_REG0A;
  reg_usb2_0B = UTMI_BASE + UTMI_USB2_REG0B;

  reg_usb2_0A_val = readl(reg_usb2_0A);
  reg_usb2_0B_val = readl(reg_usb2_0B);

  if(reg_usb2_0A_val != 0 || reg_usb2_0B_val != 0x3000) {
    writel(0, reg_usb2_0A);
    writel(USB2_REG0B_DEFAULT_VAL, reg_usb2_0B);
  }
}
/*---------------------------------------------------------------------------*/
static void
set_preamble_mode(void)
{
  usb_check_reg_before_init();
  smux_running_mode_set(SMUX_MODE_PREAMBLE);
}
/*---------------------------------------------------------------------------*/
#if AROM_VERSION_CRANE_SERIES
static bool
flash_check_qspi2_nor_device(const void *data, size_t size)
{
  size_t flash_info_size = 0;
  struct flash_info *info;

  do {
    info = (struct flash_info *)data;
    if(info->magic != FLASH_INFO_MAGIC) {
      return false;
    }
    size_t len = sizeof(struct flash_info);
    len += (info->compatible_device_num - 1) * sizeof(struct compatible_device);
    flash_info_size += len;
    data = (char *)data + len;
    if(flash_info_size > size) {
      return false;
    }
    if(!strcmp(info->device_type, FLASH_TYPE_QSPI2_NOR)) {
      return true;
    }
  } while(info->chained);

  return false;
}
#endif
/*---------------------------------------------------------------------------*/
static int
do_boot(void)
{
  smux_running_mode_set(SMUX_MODE_BYPASS);

#if AROM_VERSION_CRANE_SERIES
  /* asic-sv evb case */
  if(do_boot_pz1()) {
    LOG_ERR("can not boot pz1!\n");
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }

  extern char __lzma_alloc_base, __lzma_alloc_size;
  uint8_t *stash = (uint8_t *)((uintptr_t)&__lzma_alloc_base + SZALLOC_OFFSET);
  ssize_t stash_size = (uintptr_t)&__lzma_alloc_size - SZALLOC_OFFSET;
  stash_size = sb_volume_read("flashinfo", stash, stash_size);
  if(stash_size < 0) {
    LOG_ERR("can not read volume 'flashinfo'\n");
    set_preamble_mode();
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }

#ifdef USE_ARB
  if(is_valid_fip(stash)) {
    int ret = fip_get_image(stash, BL2_IMAGE_ID, (uint64_t *)&stash, NULL);
    if(ret < 0) {
      LOG_ERR("Get 'flashinfo' failed!\n");
      return SYS_BOOT_MODE_FORCE_DOWNLOAD;
    }
  }
#endif

  if(flash_check_qspi2_nor_device(stash, stash_size)) {
    slt_do_boot_prepare();
    LOG_INFO("Refresh volume for booting from qspi2.\n");
    sb_volume_exit();
    if(sb_volume_init() < 0) {
      LOG_ERR("Initialize volume partition failed!\n");
      set_preamble_mode();
      return SYS_BOOT_MODE_FORCE_DOWNLOAD;
    }

    /* Reload new ptable */
    if(ptable_loader()) {
      LOG_ERR("Load ptable failed!\n");
      set_preamble_mode();
      return SYS_BOOT_MODE_FORCE_DOWNLOAD;
    }
  }
#elif AROM_VERSION_CRANEW_SERIES
  /* asic-sv evb case */
  if(do_boot_pz1()) {
    LOG_ERR("can not boot pz1!\n");
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }
#endif

  if(!do_boot_cb()) {
    set_preamble_mode();
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }

  return 0;
}
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
/*---------------------------------------------------------------------------*/
PROCESS(boot2_process, "Crane boot2 process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(boot2_process, ev, data)
{
  (void)data;
  static struct etimer timer;
  static bool timer_started;

  PROCESS_BEGIN();

  /* setup a periodic timer that expires after 1 seconds. */
  LOG_PRINT("Waiting for %d ms to detect download command.\n", DOWNLOAD_CMD_DETECT_WAIT_MSECONDS);
  smux_register_preamble_callback(boot2_preamble_callback);
  etimer_set(&timer, CLOCK_SECOND * DOWNLOAD_CMD_DETECT_WAIT_MSECONDS / 1000);
  timer_started = true;

#ifdef BOARD_CRANE
  uart_rx_fixup();
#endif

  smux_running_mode_set(SMUX_MODE_PREAMBLE);

  while(1) {
    PROCESS_YIELD();

    switch(ev) {
    case PROCESS_EVENT_TIMER:
      if(smux_running_mode_get() == SMUX_MODE_PREAMBLE) {
        smux_running_mode_set(SMUX_MODE_BYPASS);
        char r_cmd;
        pmic_get_restart_cmd(&r_cmd);
        pmic_set_restart_status(r_cmd);
        LOG_INFO("No download command detected, continue booting.\n");
        if(timer_started) {
          etimer_stop(&timer);
        }
        if(!do_boot()) {
          timer_started = false;
        } else {
          set_preamble_mode();
        }
      }
      break;

    case PREAMBLE_DOWNLOAD:
      /* stop timer */
      if(timer_started) {
        etimer_stop(&timer);
        timer_started = false;
      }
      break;

    case PREAMBLE_PRODUCTION:
      LOG_INFO("PROD CMD ACCEPTED!\n");
      smux_running_mode_set(SMUX_MODE_BYPASS);
      smux_use_device(SMUX_DEV_UART);
      /* set production mode flag */
      pmic_set_restart_status(RESTART_STATUS_PRODUCTION);
      sys_boot_mode_set(SYS_BOOT_MODE_PRODUCTION);
      if(timer_started) {
        etimer_stop(&timer);
      }
      if(!do_boot()) {
        timer_started = false;
      } else {
        set_preamble_mode();
      }
      break;
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
crane_boot2_init(void)
{
  process_start(&boot2_process, NULL);
}
/*---------------------------------------------------------------------------*/
static void
transfer_arom_do_download(void)
{
  /* unregister preamble callback first */
  smux_register_preamble_callback(NULL);
  process_post_synch(arom_main_process(), PREAMBLE_DOWNLOAD, NULL);
  process_exit(&boot2_process);
#if (defined(BOARD_CRANE))
#if AROM_VERSION <= AROM_VER_2021_10_16_CRANEL_A0
  size_t len = strlen(boardid_string);
  char *p = arom_heapmem_alloc(len + 1);  /* must use arom heap */
  strcpy(p, boardid_string);
  fastboot_publish("boardid", p);
#endif
#endif
}
/*---------------------------------------------------------------------------*/
static void
boot2_preamble_callback(const char *preamble, smux_dev_t dev)
{
  static clock_time_t saved_tick;
  static enum { PREAMBLE_TYPE_UNKNOWN,
                PREAMBLE_TYPE_UUUU,
                PREAMBLE_TYPE_UABT,
                PREAMBLE_TYPE_PROD } preamble_type;

  bool first_time = false;
  if(!strncmp(preamble, SMUX_PREAMBLE_UUUU, SMUX_PREAMBLE_SIZE)) {
    if(preamble_type != PREAMBLE_TYPE_UUUU) {
      first_time = true;
      preamble_type = PREAMBLE_TYPE_UUUU;
    }
  } else if(!strncmp(preamble, SMUX_PREAMBLE_UABT, SMUX_PREAMBLE_SIZE)) {
    if(preamble_type != PREAMBLE_TYPE_UABT) {
      first_time = true;
      preamble_type = PREAMBLE_TYPE_UABT;
    }
  } else if(!strncmp(preamble, SMUX_PREAMBLE_PROD, SMUX_PREAMBLE_SIZE)) {
    if(preamble_type != PREAMBLE_TYPE_PROD) {
      first_time = true;
      preamble_type = PREAMBLE_TYPE_PROD;
    }
  } else {
    preamble_type = PREAMBLE_TYPE_UNKNOWN;
    return;
  }

  clock_time_t cur_tick = clock_time();

  if(!first_time && (cur_tick - saved_tick) < CLOCK_SECOND / 2) {
    return;
  } else {
    saved_tick = cur_tick;
  }

  switch(preamble_type) {
  case PREAMBLE_TYPE_UUUU:
    smux_use_device(dev);
    smux_write_stdout((uint8_t *)SMUX_PREAMBLE_UABT, SMUX_PREAMBLE_SIZE);
    process_post(&boot2_process, PREAMBLE_DOWNLOAD, NULL);
    break;
  case PREAMBLE_TYPE_UABT:
    smux_use_device(dev);
    /* handle download by bootrom */
    transfer_arom_do_download();
    smux_write_stdout((uint8_t *)SMUX_PREAMBLE_UABT, SMUX_PREAMBLE_SIZE);
    break;
  case PREAMBLE_TYPE_PROD:
    smux_use_device(dev);
    process_post(&boot2_process, PREAMBLE_PRODUCTION, NULL);
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* if ptable has pz1 img, boot pz1 */
static int
do_boot_pz1(void)
{
  struct ptentry *entry = ptable_find_entry("pz1");

  /* if found 'pz1' partition boot to 'pz1' partition's xip address */
  if(entry) {
    LOG_INFO("Found pz1 partition, just load and boot to %p\n",
             (void *)(uintptr_t)entry->vstart);
    if(entry->vstart == (uintptr_t)&__lzma_decode_start) {
      if(sb_flash_read(entry->start, &__lzma_decode_start, entry->size) != (int)entry->size) {
        return -1;
      }
    }
    boot_xip_address(entry->vstart);
    /* never return */
  }

  return 0;
}
#endif

/*---------------------------------------------------------------------------*/
int
main(void)
{
  LOG_PRINT("Executing boot2 application...\n");
  LOG_PRINT("Boot2 version: %s\n", BOOTLOADER_VERSION);

  if(arom_production_mode_get() == 1) {
    /* set production mode flag */
#if (AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5) \
    && (AROM_VERSION != AROM_VER_2022_07_28_CRANEW_Z2)
    pmic_set_restart_status(RESTART_STATUS_PRODUCTION);
#endif
    sys_boot_mode_set(SYS_BOOT_MODE_PRODUCTION);
  }

  sys_boot_mode_t boot_mode = sys_boot_mode_get();

  if(ptable_loader()) {
    LOG_ERR("Load ptable failed!\n");
    return -1;
  }

#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
#if (defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU))
  boardid_string = volume_read_boardid();
  asr_property_set(PROP_NAME_BOARDID, boardid_string);
  LOG_INFO("BOARD_ID: %s\n", boardid_string);
#endif
#endif

  /*
   * can not know whether usb-vbus is connected or not,
   * try download mode only support usb-vbus connected case,
   * if usb-vbus not connected, customer use at+qdownload = 1 to enter force download mode
   */
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
  if(!usb_connect()) {
    LOG_WARN("usb is not connected\n");
    if(boot_mode == SYS_BOOT_MODE_TRY_DOWNLOAD) {
#if DISABLE_UART_TRY_DOWNLOAD
      boot_mode = SYS_BOOT_MODE_NORMAL;
#endif
    }
  }
#endif

  switch(boot_mode) {
  case SYS_BOOT_MODE_NORMAL:
    LOG_INFO("Running in normal boot mode.\n");
    return do_boot();
  case SYS_BOOT_MODE_PRODUCTION:
    LOG_INFO("Running in production mode.\n");
    return do_boot();
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
  case SYS_BOOT_MODE_TRY_DOWNLOAD:
    LOG_INFO("Running in try downloading mode.\n");
    crane_boot2_init();
    return 0;
#endif
  case SYS_BOOT_MODE_FORCE_DOWNLOAD:
    LOG_INFO("Running in force downloading mode.\n");
#if AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0
    /* only for AROM_VER_2020_07_30_CRANEGM_A0 */
    /* an patch for arom-crane 0730
     * fix bug for AT fall back invalid.
     * arom-crane miss judge ret from boot2,  so use negative value.
     */
    return -SYS_BOOT_MODE_FORCE_DOWNLOAD;
#else
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
#endif

  default:
    break;
  }

  return -1;
}
