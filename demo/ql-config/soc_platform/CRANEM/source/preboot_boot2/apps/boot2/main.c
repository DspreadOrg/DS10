#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <byteorder.h>
#include <ptable.h>
#include "reg.h"
#include "secureboot.h"
#include "pmic.h"
#include "sys-boot-mode.h"
#include "sys_close_all.h"
#include "smux.h"
#include "etimer.h"
#include "clock.h"
#include "arom-crane.h"
#include "lzma.h"
#include "pmu.h"
#include "system.h"
#include "property.h"
#include "arom_patch_auth.h"
#include "syscall-arom.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE "Boot2"
#define LOG_LEVEL  LOG_LEVEL_MAIN
/*---------------------------------------------------------------------------*/
#define DOWNLOAD_CMD_DETECT_WAIT_MSECONDS 1000 /* ms */
/*---------------------------------------------------------------------------*/
/**
 * Preamble message type
 */
enum {
  PREAMBLE_DOWNLOAD,
  PREAMBLE_PRODUCTION
};
/*---------------------------------------------------------------------------*/
static void crane_boot2_init(void);
static void boot2_preamble_callback(const char *preamble, smux_dev_t dev);
static void transfer_arom_do_download(void);
/*---------------------------------------------------------------------------*/
static sys_boot_mode_t sys_boot_mode;
/*---------------------------------------------------------------------------*/
extern unsigned char __scratch2_start, __scratch2_end;
/*---------------------------------------------------------------------------*/
/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID            1

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID           5

#define BOOT33_ADDRESS          0x7E000000
#define BOOT33_MAX_SIZE         0x100000
#define BOOT33_DECODE_ADDRESS   (BOOT33_ADDRESS + BOOT33_MAX_SIZE)

#define QSPI_AMBA_BASE          0x80000000U
#define QSPI_IPB_BASE           0xD420B000
#define QSPI_FLASH_A1_OFFSET    0x00000000
#define QSPI_SFAR               0x100
#define QSPI_BFGENCR            0x020
#define FLASH_TO_MEMORY_OFFSET (QSPI_AMBA_BASE + QSPI_FLASH_A1_OFFSET)
/*---------------------------------------------------------------------------*/
/**
 * Get system boot mode
 *
 * \return current boot mode.
 */
static sys_boot_mode_t
sys_boot_mode_get(void)
{
  return sys_boot_mode;
}
/*---------------------------------------------------------------------------*/
/**
 * Set system boot mode
 *
 * \param boot_mode The new boot mode will be set
 */
static void
sys_boot_mode_set(sys_boot_mode_t boot_mode)
{
  sys_boot_mode = boot_mode;
}
/*---------------------------------------------------------------------------*/
__attribute__((constructor)) static void
platform_init(void)
{
  /*
   * if usb is not connected, forbid to LOG_INFO;
   * if usb connected, permit LOG_INFO.
   */
  if(usb_connect() == 0) {
    log_set_level("main", LOG_LEVEL_WARN);
  }

#ifdef CPU_MODEL_CORTEXR
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
}
/*---------------------------------------------------------------------------*/
static int
set_xip(void)
{
  writel(0, QSPI_IPB_BASE + QSPI_BFGENCR);
  writel(FLASH_TO_MEMORY_OFFSET, QSPI_IPB_BASE + QSPI_SFAR);

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
boot_image(uintptr_t offset)
{
  transfer_parameter param;

  param.pc = offset;
  param.spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);

  LOG_INFO("Boot to 0x%x, cpsr should be 0x%x\n", (unsigned int)param.pc, (unsigned int)param.spsr);

  /* arom_sys_shutdown(); */
  sys_close_all();

  transfer_control(&param);
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
static int
ptable_loader(void)
{
  uint8_t *stash;
  ssize_t stash_size;
  ssize_t vol_size;

  /*
   * read ptable to __scratch2_start
   */
  stash = &__scratch2_start;
  stash_size = &__scratch2_end - &__scratch2_start;
  LOG_INFO("Loading partition table into %p\n", stash);
  vol_size = sb_volume_read("partition", stash, stash_size);
  if(vol_size < 0) {
    LOG_ERR("can not read volume 'partition'\n");
    return 1;
  }
  if(ptable_init(stash, vol_size) < 0) {
    LOG_ERR("Partition table init failed\n");
    return 1;
  }
  struct ptentry *ptable = ptable_find_entry("ptable");
  if(!ptable) {
    LOG_ERR("Can't find ptable partition in partition table!\n");
    return 1;
  }
  vol_size = ptable->vsize;
  if(vol_size != sb_flash_read(ptable->start, stash, vol_size)) {
    LOG_ERR("Read ptable from flash failed!\n");
    return 1;
  }
  if(ptable_init((const void *)stash, vol_size) < 0) {
    LOG_WARN("New partition table init failed!!!\n");
    LOG_WARN("Will use the old partition table instead!!!\n");
    sb_volume_read("partition", stash, stash_size);
  }

  /* dump ptable */
  ptable_dump();
  LOG_INFO("Done.\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
boot2_loader(void)
{
  if(set_xip() < 0) {
    LOG_ERR("Set XIP failed!\n");
    return 1;
  }

#if 0
  struct ptentry *fwcerts = ptable_find_entry("fwcerts");
  if(!fwcerts) {
    LOG_ERR("Can't find fwcerts partition in partition table!\n");
    return 1;
  }

  display_binary((void *)(FLASH_TO_MEMORY_OFFSET + fwcerts->start), 256);
#endif

  uint8_t *boot33_start_addr = (uint8_t *)BOOT33_ADDRESS;
  uint8_t *boot33_decode_addr = (uint8_t *)BOOT33_DECODE_ADDRESS;

  /* load boot33 */
  LOG_INFO("Load and verify boot33 from bootloader volume.\n");
  ssize_t vol_size = sb_volume_read("bootloader", boot33_start_addr, BOOT33_MAX_SIZE);
  if(vol_size < 0) {
    LOG_ERR("can not read volume 'bootloader'\n");
    return 1;
  }

  /*arom_patch_auth must be invoked when do boot, cannot be invoke when boot2 downloading */
  if(AROM_VER_2020_07_30 == arom_getversion()) {
    /* apply arom code patch */
    arom_patch_auth();
  }

  if(sb_auth_image(BL33_IMAGE_ID, boot33_start_addr, boot33_start_addr) < 0) {
    LOG_ERR("verify boot33 failed\n");
    return 1;
  }
  LOG_INFO("Done.\n");

  /* try lzma decode boot33 */
  lzma_props_t props;
  if(!lzmaprops_decode(&props, boot33_start_addr, vol_size)) {
    if(props.lc == 3 &&
       props.lp == 0 &&
       props.pb == 1 &&
       props.dicSize == 4096) {
      LOG_INFO("Decompressing boot33...\n");
      size_t srcLen = (size_t)vol_size;
      size_t destLen = BOOT33_MAX_SIZE;
      int rc;
      if((rc = lzma_decode(boot33_decode_addr, &destLen, boot33_start_addr, &srcLen))) {
        LOG_ERR("Decompress failed, status = %d, srcLen=0x%x, destLen=0x%x\n", rc, (unsigned)srcLen, (unsigned)destLen);
        return 1;
      } else {
        memmove(boot33_start_addr, boot33_decode_addr, destLen);
      }
      LOG_INFO("Done.\n");
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
boot2_boot_boot33(void)
{
  if(ptable_loader()) {
    LOG_ERR("Load ptable failed!\n");
    return 1;
  }

  asr_property_set("bl2.boot2.ver", BOOTLOADER_VERSION);

  LOG_INFO("Booting boot33...\n");
  boot_image(BOOT33_ADDRESS);

  return 0;
}
/*---------------------------------------------------------------------------*/
static int
do_boot(void)
{
  smux_running_mode_set(SMUX_MODE_BYPASS);
  if(!boot2_loader() && !boot2_boot_boot33()) {
    /* never return */
    return 0;
  } else {
    LOG_WARN("Load and run boot33 failed, fall back into downloading mode.\n");
    smux_running_mode_set(SMUX_MODE_PREAMBLE);
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }
}
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
          transfer_arom_do_download();
        }
      }
      break;

    case PREAMBLE_PRODUCTION:
      LOG_INFO("PROD CMD ACCEPTED!\n");
      smux_running_mode_set(SMUX_MODE_BYPASS);
      /* set production mode flag */
      pmic_set_restart_status(RESTART_STATUS_PRODUCTION);
      if(timer_started) {
        etimer_stop(&timer);
      }
      if(!do_boot()) {
        timer_started = false;
      } else {
        transfer_arom_do_download();
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
  smux_running_mode_set(SMUX_MODE_PREAMBLE);
  process_post(arom_main_process(), PREAMBLE_DOWNLOAD, NULL);
  process_exit(&boot2_process);
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
    /* handle download by bootrom */
    transfer_arom_do_download();
    break;
  case PREAMBLE_TYPE_UABT:
    if(smux_running_mode_get() == SMUX_MODE_PREAMBLE) {
      smux_use_device(dev);
      transfer_arom_do_download();
    }
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
int
main(void)
{
  LOG_PRINT("Executing boot2 application...\n");
  LOG_PRINT("Boot2 version: %s\n", BOOTLOADER_VERSION);

  sys_boot_mode_t boot_mode = sys_boot_mode_get();
  if(!usb_connect()) {
    LOG_INFO("usb is not connected\n");
    if(boot_mode == SYS_BOOT_MODE_TRY_DOWNLOAD) {
#if DISABLE_UART_TRY_DOWNLOAD
      boot_mode = SYS_BOOT_MODE_NORMAL;
#endif
    }
  }

  switch(boot_mode) {
  case SYS_BOOT_MODE_NORMAL:
    LOG_INFO("Running in normal boot mode.\n");
    return do_boot();
  case SYS_BOOT_MODE_PRODUCTION:
    LOG_INFO("Running in production mode.\n");
    return do_boot();
  case SYS_BOOT_MODE_TRY_DOWNLOAD:
    LOG_INFO("Running in try downloading mode.\n");
    crane_boot2_init();
    return 0;
  case SYS_BOOT_MODE_FORCE_DOWNLOAD:
    LOG_INFO("Running in force downloading mode.\n");
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;

  default:
    break;
  }

  return -1;
}
