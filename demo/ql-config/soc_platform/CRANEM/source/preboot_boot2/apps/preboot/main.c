#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "psram.h"
#include "i2c.h"
#include "pmic.h"
#include "freq.h"
#include "cpu.h"
#include "secureboot.h"
#include "aboot.h"
#include "reg.h"
#include "smux.h"
#include "property.h"
#include "efuse.h"
#include "syscall-arom.h"
#include "gpio.h"
#include "log.h"
#define LOG_MODULE "Preboot"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define BOOT33_ADDRESS          0x7E000000
#define BOOT33_MAX_SIZE         0x100000
/*---------------------------------------------------------------------------*/
#define UMTI_BASE                 0xD4207000
#define USB_PHY_REG_28            0xA0
#define USB_PHY_REG_29            0xA4
#define HS_ISEL_MASK              (0x3 << 12)
#define HS_ISEL                   (0x3 << 12)
/*---------------------------------------------------------------------------*/
/* AROM_VER_2020_07_30 */
#if 1
#include "sys-boot-mode.h"

/*only for AROM_VER_2020_07_30 */
/* an patch for arom-crane 0730
 * fix bug for AT fall back invalid.
 * arom-crane miss judge ret from boot2, but has judge ret from preboot, so use preboot to judge sys_boot_mode.
 */
static int
arom_patch_for_force_download_2020_7_30(void)
{
#ifdef CPU_MODEL_CORTEXR
  char r_cmd;

  if(AROM_VER_2020_07_30 != arom_getversion()) {
    return 0;
  }

  pmic_init();
  pmic_get_restart_cmd(&r_cmd);
  if(r_cmd == RESTART_CMD_USB_DOWNLOAD) {
    pmic_set_restart_cmd(RESTART_CMD_NORMAL_POWERON);
    LOG_PRINT("Running in reboot to download mode.\n");
    return SYS_BOOT_MODE_FORCE_DOWNLOAD;
  }
#endif
  return 0;
}
#endif
/*---------------------------------------------------------------------------*/
void
usb_phy_config(void)
{
  unsigned val;
  unsigned chip_id;

  chip_id = hw_chip_id();
  if (chip_id == CHIP_ID_CRANE) {
    /* driver strength */
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_28);
    writel((val & ~HS_ISEL_MASK) | HS_ISEL, UMTI_BASE + USB_PHY_REG_28);
    LOG_PRINT("Usb phy reg 28 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_28));
  } else if ((CHIP_ID_CRANEG == chip_id) || (CHIP_ID_CRANEM == chip_id)) {
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_29);
    writel((val & ~0x1F) | 0x1B, UMTI_BASE + USB_PHY_REG_29);
    LOG_PRINT("Usb phy reg 29 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_29));
  }
}
/*---------------------------------------------------------------------------*/
int
main(void)
{
	/*Fias 2021.08.05  更改1603系列，配置gpio8(指示灯)输出为低；preboot version: 2021.08.05*/
	gpio_t  gpio_num = 8;
	gpio_mode_t  gpio_mode = GPIO_OUT;
	gpio_init(gpio_num, gpio_mode);
	gpio_clear(gpio_num); 	  // Default output of GPIO8 is set to low
  /*
   * if preboot when booting(SMUX_MODE_BYPASS), forbid to LOG_INFO;
   * if preboot when downloading(SMUX_MODE_NORMAL), permit LOG_INFO.
   */
  if(smux_running_mode_get() == SMUX_MODE_BYPASS) {
    /* forbid LOG_INFO */
    log_set_level("main", LOG_LEVEL_WARN);
  }

  LOG_PRINT("Executing preboot application...\n");
  LOG_PRINT("Preboot version: %s\n", BOOTLOADER_VERSION);

  /* force init prop 1K space: 0XB0020C00~ 0xB0021000 */
  asr_property_area_init(1);
  asr_property_set("bl2.preboot.ver", BOOTLOADER_VERSION);

#ifdef CPU_MODEL_CORTEXR
  unsigned int chip_id;

  /* arom-crane alread print log, this is only for set prop */
  if(!fuse_trust_boot_enabled()) {
    /*LOG_PRINT("### Non-trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "0");
  } else {
    /*LOG_PRINT("### Trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "1");
  }

  /*
   * "halt": When download completed, if usb pull out, board powerdown. defaultly it is "halt",
   * "reboot": some customer want to reboot instead of powerdown, so customer set it to "reboot" manually.
   */
  asr_property_set("bl2.usb_disconnect", "halt");

  usb_phy_config();

  fuse_read_lotid();

  chip_id = readl(CIU_BASE + 0) & 0xFFFF;
  /*LOG_INFO("\nCHIP_ID: 0x%04X\n", chip_id); */
  if(hw_platform_type() == HW_PLATFORM_TYPE_SILICON) {
    pmic_setup();
    uint8_t powerup_reason = pmic_powerup_get_reason();
    LOG_PRINT("Power_up_reason=0x%x.\n", powerup_reason);

    pmic_fault_wu_en_disable();
    /* except bit1 and bit2 */
    if(!(powerup_reason & 0x79)) {
      LOG_WARN("Warning: power_up_reason unknown.\n");
      /* pmic_sw_pdown(); */
      /* return 0; */
    }

    /* workaround for 1601+PM802, if power up log = 0, pmic power down. PM813 no this hardware bug. */
    if(0 == powerup_reason) {
      LOG_WARN("Warning: power_up_reason = 0. need pmic power down\n");
      pmic_sw_pdown();
      return 0;
    }
  }

  pmic_set_buck_vol(chip_id);

  cr5_axi_set_max_freq();
#endif

  psram_init();

  /*
   * Atcm 64K is not enough to parse bootloder volume, so update to psram.
   */
  LOG_INFO("Set fip image load address spaces to 0x%x\n", BOOT33_ADDRESS);
  sb_fip_image_setup((void *)BOOT33_ADDRESS, BOOT33_MAX_SIZE);

  if(AROM_VER_2020_07_30 == arom_getversion()) {
    if(smux_running_mode_get() == SMUX_MODE_BYPASS) {
      int ret = arom_patch_for_force_download_2020_7_30();
      if(ret != 0) {
        return ret;
      }
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
