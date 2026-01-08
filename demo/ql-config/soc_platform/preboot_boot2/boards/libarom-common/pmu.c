/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
/*---------------------------------------------------------------------------*/
#if defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU)
#include <stdbool.h>
#include "contiki.h"
#include "pmu.h"
#include "reg.h"
#include "cpu.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE "PMU"
#define LOG_LEVEL  LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define PMU_BASE                               0xD4282800
/* usb detect*/
#define PMU_SD_ROT_WAKE_CLR                    (PMU_BASE + 0x7C)
#define USB_VBUS_WK_SRC_SEL                    (1 << 8)
#define VBUS_DETECT                            (1 << 15)
#define VBUS_DETECT_MUX                        (1 << 17)
#define VBUS_DETECT_MUX_WATCH1                 (1 << 27)
/*---------------------------------------------------------------------------*/
static bool
usb_support_phy_vbus_detect(void)
{
#if (AROM_VERSION < AROM_VER_2022_03_31_SC2_A0)
  return false;
#else
  uint32_t chip_id = hw_chip_id();
  switch(chip_id) {
  case CHIP_ID_CRANELS:
  case CHIP_ID_CRANEW:
  case CHIP_ID_FULMAR:
  case CHIP_ID_CRANELR:
  case CHIP_ID_CRANEGT:
  case CHIP_ID_CRANELG:
  case CHIP_ID_CRANELRH:
  case CHIP_ID_LAPWING:
    return false;
  }

  return true;
#endif
}
/*---------------------------------------------------------------------------*/
/*
 * if USB_VBUS is connected(R1101(12K) and R1104(6.8K) is connected,
 *   USB_DETECT is connected to USB_VBUS),
 * if USB_VBUS is unconnected, bit VBUS_DETECT always 0.
 */
int
usb_connect_with_usb_detect(void)
{
  uint32_t val = readl(PMU_SD_ROT_WAKE_CLR);
  uint32_t det_mask;

  if(usb_support_phy_vbus_detect()) {
    if((val & USB_VBUS_WK_SRC_SEL) == 0x0) {
      writel(USB_VBUS_WK_SRC_SEL | val, PMU_SD_ROT_WAKE_CLR);
    }
  }

  val = readl(PMU_SD_ROT_WAKE_CLR);
  LOG_DBG("PMU_SD_ROT_WAKE_CLR(0x%x) = 0x%" PRIx32 "\n", PMU_SD_ROT_WAKE_CLR, val);

  /**
   * Those chips (CraneLB and later) have 2 schemes for VBUS detection:
   * 1. VBUS valid form GPIO(legacy).
   * 2. VBUS valid from PHY(newly added).
   * Apply the PHY scheme by default, and the free GPIO can be used
   * for another purpose.
   */
  if(usb_support_phy_vbus_detect()) {
    det_mask = (hw_chip_id() == CHIP_ID_WATCH1) ? VBUS_DETECT_MUX_WATCH1 : VBUS_DETECT_MUX;
    return (val & det_mask) ? 1 : 0;
  }

  return (val & VBUS_DETECT) ? 1 : 0;
}
/*---------------------------------------------------------------------------*/
#define USB_BASE                                0xD4208000
#define USB_PORT_SC                             (USB_BASE + 0x184)
#define SUSP_DETECT                             (1 << 7)
/*---------------------------------------------------------------------------*/
/*
 * if USB_VBUS is unconnected(R1101(12K) and R1104(6.8K) is unconnected,
 *  USB_DETECT is unconnected to USB_VBUS),
 * USB_DETECT always be 0 no matter usb connect or unconnect.
 * PMU_SD_ROT_WAKE_CLR can not be use,
 * need read USB_PORT_SC, if SUSP_DETECT[bit7] = 1 means usb is unconnected.
 *
 * if downloading, bit7 = 0. if usb pull out when download over, bit7 = 1
 * Note:
 * before try download, USB_PORT_SC always is 0x0, no matter USB_VBUS connected or not.
 * so usb_connect_without_usb_detect cannot be used before received uuuu(preamble),
 * it only can be used when downloading, to check whether usb is pulled out.
 */
int
usb_connect_without_usb_detect(void)
{
  unsigned val = readl(USB_PORT_SC);

  LOG_DBG("USB_PORT_SC(0x%x) = 0x%x\n", USB_PORT_SC, val);
  return (val & SUSP_DETECT) ? 0 : 1;
}
/*---------------------------------------------------------------------------*/
/*
 * if usb-vbus not connected, customer use at+qdownload = 1 to enter force download mode,
 * not support wait 1s to try download mode.
 *
 * cr5 should not invoke usb_connect(). it is for mcu.
 */
int
usb_connect(void)
{
  if(hw_platform_type() == HW_PLATFORM_TYPE_FPGA) {
    /* HW_PLATFORM_TYPE_FPGA can not detect usb, return connected */
    return 1;
  } else if(hw_platform_type() == HW_PLATFORM_TYPE_ZEBU_Z1) {
    return 0;
  }

  return usb_connect_with_usb_detect();
}
/*---------------------------------------------------------------------------*/
/*
 * move geu clk enable to pmu.c instead of geu.c
 * to support preboot enable gen clock, boot2 disable geu clock
 * register is diff for cranew, refer to cpu.h
 */
static void
_geu_clk_enable(bool en)
{
#ifdef MCU_SYS_PMU_CTRL
  /* cranew CR5 */
  uint32_t pum_ctrl = readl(MCU_SYS_PMU_CTRL);

  if(en) {
    writel(pum_ctrl | AEU_CG_EN, MCU_SYS_PMU_CTRL);
  } else {
    writel(pum_ctrl & (~AEU_CG_EN), MCU_SYS_PMU_CTRL);
  }
#else
  pmu_aes_clk_res_ctrl_t pmu_aes_clk_res_ctrl;
  pmu_aes_clk_res_ctrl.value = readl(PMU_AES_CLK_RES_CTRL);

  if(en) {
    pmu_aes_clk_res_ctrl.s.aes_axi_clk_en = 1;
  } else {
    pmu_aes_clk_res_ctrl.s.aes_axi_clk_en = 0;
  }

  pmu_aes_clk_res_ctrl.s.aes_axi_reset = 1;
  writel(pmu_aes_clk_res_ctrl.value, PMU_AES_CLK_RES_CTRL);
#endif
}
/*---------------------------------------------------------------------------*/
void
geu_clk_enable(void)
{
  unsigned int val = readl(ACGR);

  val |= (1 << APMU_104M_BIT);
  writel(val, ACGR);

  _geu_clk_enable(1);
}
/*---------------------------------------------------------------------------*/
void
geu_clk_disable(void)
{
  _geu_clk_enable(0);
}
#endif
