/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 *
 * cr5 should not invoke interface of pmic, it only be invoked by mcu.
 * although BOARD_CRANE include cr5, but no functions will invoke pmic interface from cr5.
 */
#if defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU)
#include <stdio.h>
#include <stdint.h>
#include "pmic.h"
#include "reg.h"
#include "i2c.h"
#include "cpu.h"
#include "property.h"
#include "sys_close_all.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE "PMIC"
#define LOG_LEVEL  LOG_LEVEL_MAIN

#define BUCK_LDO_SWITCH     0x14
#define BUCK4_SW3           (1 << 6)
/*---------------------------------------------------------------------------*/
static uint8_t g_pmic_id = PMIC_UNAVAILABLE;
/*---------------------------------------------------------------------------*/
/*
 * default value 75mA, set to 150mA.
 *
 * base regs: 0x21:
 * pm813/pmic813s/pm816: valid
 * pm802/pm803   : reserved, blank.
 */
#if AROM_VERSION != AROM_VER_2021_10_16_CRANEL_A0	//cranel裁剪代码，加快开机时间 quectel
static void
pmic_pre_charge_current_setting(void)
{
  uint8_t val;
  uint8_t pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  pi2c_init(PMIC_BASE_PAGE_ADDR);

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    LOG_WARN("read pmic id failed\n");
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM813:
  case PM813S:
  case PM816:
    LOG_INFO("Set prcharge current to 150mA\n");
    pi2c_init(PMIC_BASE_PAGE_ADDR);

    pi2c_read_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, &val);
    pi2c_write_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, (val & 0xF3) | PRE_CHARGE_CURRENT_SETTING_150MA);
    pi2c_read_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, &val);
    LOG_INFO("PM816/PM813s/PM813_PRE_CHARGE_CURRENT_SETTING_REG: [0x%x] = 0x%x\n", PM813_PRE_CHARGE_CURRENT_SETTING_REG, val);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
#ifdef BOARD_CRANEW_MCU
static void
pmic_enable_fulmar_ddr_power_supply(void)
{
  uint8_t val;

  /* enable buck4_sw3 */
  pi2c_read_reg(BUCK_LDO_SWITCH, &val);
  val |= BUCK4_SW3;
  pi2c_write_reg(BUCK_LDO_SWITCH, val);
}
#endif
#endif
/*---------------------------------------------------------------------------*/
uint8_t
pmic_get_id(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return PMIC_UNAVAILABLE;
  }

  if(PMIC_UNAVAILABLE != g_pmic_id) {
    return g_pmic_id;
  }

  pi2c_init(PMIC_BASE_PAGE_ADDR);

  if(pi2c_read_reg(PMIC_ID, &val) < 0) {
    LOG_WARN("read pmic id failed\n");
    return PMIC_UNAVAILABLE;
  }

  /*LOG_INFO("PMIC id = 0x%x\n", (unsigned)val); */
  g_pmic_id = val;
  return val;
}
/*---------------------------------------------------------------------------*/
/*
 * preboot set 10pf
 * cp will set 20pf after 1s
 * pm803 do not have CRYSTAL select bits
 */
#if AROM_VERSION != AROM_VER_2021_10_16_CRANEL_A0	//cranel裁剪代码，加快开机时间 quectel
static void
pmic_set_crystal(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    LOG_WARN("read pmic id failed\n");
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM803:
  case PM803L:
  case PM807:
    break;
  case PM813L:
    LOG_INFO("Set crystal 10pf\n");
    pi2c_init(PMIC_BASE_PAGE_ADDR);
    pi2c_read_reg(PM813L_RTC_REG_XO, &val);
    pi2c_write_reg(PM813L_RTC_REG_XO, (val & 0xF8) | CRYSTAL_10PF_813L);
    pi2c_read_reg(PM813L_RTC_REG_XO, &val);
    break;
  default:
    LOG_INFO("Set crystal 10pf\n");
    pi2c_init(PMIC_BASE_PAGE_ADDR);
    pi2c_read_reg(PM813_RTC_REG_XO, &val);
    pi2c_write_reg(PM813_RTC_REG_XO, (val & 0x1F) | CRYSTAL_10PF);
    pi2c_read_reg(PM813_RTC_REG_XO, &val);
    /*LOG_INFO("[%s]: [0x%x] = 0x%x\n", __func__, PM813_RTC_REG_XO, val); */
    break;
  }
}
#endif
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0x1d:
 * pm802/803/pm813/pm813S: valid.
 * pm802: must disable watchdog by software in preboot;
 * pm813: watchdog disabeled defaultly by hardware.
 *
 */
void
pmic_setup(void)
{
  unsigned char val;
  int pmic_reset_retry = 3;
  unsigned chip_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  chip_id = hw_chip_id();
  /*LOG_INFO("\nCHIP_ID: 0x%04X\n", chip_id); */
  while(pmic_reset_retry--) {
    pi2c_bus_reset();
    pi2c_reset();
    pi2c_init(PMIC_BASE_PAGE_ADDR);
    if(pi2c_read_reg(PMIC_ID, &val) == 0) {
      break;
    }
  }

  if(pi2c_init(PMIC_BASE_PAGE_ADDR) < 0) {
    goto error;
  }

  val = pmic_get_id();
  if(val == PMIC_UNAVAILABLE) {
    goto error;
  }

  LOG_PRINT("Found PMIC with Id: 0x%x\n", (unsigned)val);
#if AROM_VERSION != AROM_VER_2021_10_16_CRANEL_A0	//cranel裁剪代码，加快开机时间 quectel
  pmic_set_crystal();
  pmic_pre_charge_current_setting();
#endif
  LOG_INFO("Disabling PMIC watchdog\n");     /* work as a delay */

  /* disable watchdog */
  if(pi2c_read_reg(PMIC_WATCHDOG, &val) < 0) {
    goto error;
  }

  if(pi2c_write_reg(PMIC_WATCHDOG, val | WATCHDOG_WD_DIS) < 0) {
    goto error;
  }

  val = 0;
  if(pi2c_read_reg(PMIC_WATCHDOG, &val) < 0) {
    goto error;
  }

  if(val & WATCHDOG_WD_DIS) {
    LOG_PRINT("PMIC watchdog is disabled, reg val = 0x%x\n", val);
  } else {
    LOG_ERR("Disable watchdog failed, reg val = 0x%x\n", val);
  }

  /* pm803 need, before read or write 0xE7 and 0x10 */
  pmic_print_fault_event_info_for_pm803();

  pmic_set_buck1_vol(chip_id);
  pmic_set_buck1_skipmode(chip_id);
#if AROM_VERSION != AROM_VER_2021_10_16_CRANEL_A0	//cranel裁剪代码，加快开机时间 quectel
  if(CHIP_ID_CRANEM == chip_id) {
    pmic_set_ldo7_en_1_6v_for_craneM_and_pm803();
  }

  if(CHIP_ID_CRANEGT == chip_id) {
    pmic_set_ddr_phy_voltage();
  }
#ifdef QUECTEL_PREBOOT_OPT
  //pmic807,射频开关供电与USB供电共用,开关规格书供电最大是3.1V,pmic默认电压是3.1V，有很大风险，将供电电压降到3V
  if((CHIP_ID_CRANELG == chip_id)||(CHIP_ID_CRANELRH == chip_id)) {
    pmic_set_ldo4_en_3v_for_craneLG_LRH_and_pm807();
  }
#endif
#endif
#ifdef BOARD_CRANEW_MCU
  if(hw_chip_id() == CHIP_ID_FULMAR) {
    pmic_enable_fulmar_ddr_power_supply();
  }
  pmic_LDO_config_for_cranew_mcu_and_pm813s();
#endif

  return;
error:
  LOG_ERR("pmic_setup failed\n");
}
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0x10
 * pm802/803/pm813/pm813S: valid
 * pm802/803/pm806:
 * bit 7       6             5               4                 3             2                 1                0
 *       |             |              | fault_wu_log     | bat_wu_log | rtc_alarm_wu_log | exton1n_wu_log | onkey_wu_log |
 * pm813/pm813S:
 *
 *       |ivbus_detect | fault_wu_log | rtc_alarm_wu_log | bat_wu_log | exton2_wu_log    | exton1n_wu_log | onkey_wu_log |
 * pm802S:
 *
 *                     | fault_wu_log | rtc_alarm_wu_log | bat_wu_log | exton2_wu_log    | exton1n_wu_log | onkey_wu_log |
 * normal evb board: never connect exton2 or exton1n to GND.
 * user special auto test need: temperally connect exton1n to GND.
 *
 */
uint8_t
pmic_powerup_get_reason(void)
{
  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    LOG_INFO("Fake: pmic_powerup_get_reason: 0x1\n");
    return 0x1;
  }

  uint8_t powerup_reason;

  pi2c_init(PMIC_BASE_PAGE_ADDR);
  pi2c_read_reg(PM813_PWRUP_LOG_REG1, &powerup_reason);

  return powerup_reason;
}
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0xe7
 * PM802/PM803/PM813/PM813S: valid
 *
 * PM802 fault_wu_en bit2 is diff with PM803/PMIC813/PMIC813s bit0
 *
 * Bit 7  6  5   4       3           2             1         0
 * PM813
 *                    | fault_wu  |             |          | fault_wu_en |
 * PM803/PM813S
 *                                              | fault_wu | fault_wu_en |
 * PM802/pm802S
 *                    | fault_wu  | fault_wu_en |          | bat_wu_done |
 */
void
pmic_fault_wu_en_disable(void)
{
  uint8_t val;
  uint8_t pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  LOG_INFO("Disable PMIC fault wakeup.\n");
  pi2c_init(PMIC_BASE_PAGE_ADDR);

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    LOG_WARN("read pmic id failed\n");
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM802:
  case PM802S:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val &= ~PM802_FAULT_WU_ENABLE_BIT;
    break;
  default:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val &= ~PM813_FAULT_WU_ENABLE_BIT;
    break;
  }

  pi2c_write_reg(PM813_FAULT_WU_REG, val);
}
/*---------------------------------------------------------------------------*/
void
pmic_fault_wu_en_enable(void)
{
  uint8_t val;
  uint8_t pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  LOG_INFO("Enable PMIC fault wakeup.\n");
  pi2c_init(PMIC_BASE_PAGE_ADDR);

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    LOG_WARN("read pmic id failed\n");
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM802:
  case PM802S:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val |= PM802_FAULT_WU_ENABLE_BIT;
    break;
  default:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val |= PM813_FAULT_WU_ENABLE_BIT;
    break;
  }

  pi2c_write_reg(PM813_FAULT_WU_REG, val);
}
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0xe7
 * PM802/PM803/PM813/PM813S: valid
 *
 * PM802 fault_wu_en bit2 is diff with PM803/PMIC813/PMIC813s bit0
 * PM813/PM802 fault_wu bit3 is diff with PM803/PMIC813s bit1
 *
 * Bit 7  6  5   4       3           2             1         0
 * PM813
 *                    | fault_wu  |             |          | fault_wu_en |
 * PM803/PM813S
 *                                              | fault_wu | fault_wu_en |
 * PM802/pm802S
 *                    | fault_wu  | fault_wu_en |          | bat_wu_done |
 */
void
pmic_set_fault_wu(void)
{
  uint8_t val;
  uint8_t pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  LOG_INFO("Set PMIC fault wakeup.\n");
  pi2c_init(PMIC_BASE_PAGE_ADDR);

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    LOG_WARN("read pmic id failed\n");
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM802:
  case PM802S:
  case PM813:
  case PM813A3:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val |= PM813_FAULT_WU_BIT;
    break;
  default:
    pi2c_read_reg(PM813_FAULT_WU_REG, &val);
    val |= PM813S_FAULT_WU_BIT;
    break;
  }

  pi2c_write_reg(PM813_FAULT_WU_REG, val);
}
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0xd
 * PM802/PM803/PM813/PM813S: valid
 *
 * sw_pdown bit5
 * pm802/PM803/PMIC813/PMIC813s: same
 *
 * Bit 7       6            5           4         3                  2                    1           0
 * pm802/pm802S
 * pwr_hold | pmic_reset | sw_pdown |	wd_reset |disable_pgood_det	|                   |            | wd_mode
 * PM803/PMIC813/PMIC813s
 * pwr_hold | pmic_reset | sw_pdown |	wd_reset |disable_pgood_det	| force_i2c_control |            | wd_mode
 *
 */
void
pmic_set_sw_powerdown(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  LOG_INFO("Set PMIC powerdown.\n");
  pi2c_init(PMIC_BASE_PAGE_ADDR);
  pi2c_read_reg(PM813_MISC_CFG_REG1, &val);
  pi2c_write_reg(PM813_MISC_CFG_REG1, val | PM813_SW_PDOWN);
}
/*---------------------------------------------------------------------------*/
/* only pm803 bug:
 * if read or wirte  0xE7, 0xE5 and 0xE6(shutdown reason) are cleared automatically
 * so print the value for development step.
 */
void
pmic_print_fault_event_info_for_pm803(void)
{
  uint8_t val;
  char buf[4];

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM803:
  case PM803L:
    /* save the value to properties because there regs are read to clear */
    pi2c_init(PMIC_BASE_PAGE_ADDR);
    pi2c_read_reg(PM813_FAULT_EVENT_REG1, &val);
    LOG_PRINT("[BP_0x%x] = 0x%x\n", PM813_FAULT_EVENT_REG1, val);
    snprintf(buf, sizeof(buf), "%d", val);
    asr_property_set("bl2.pdreason1", buf);
    pi2c_read_reg(PM813_FAULT_EVENT_REG2, &val);
    LOG_PRINT("[BP_0x%x] = 0x%x\n", PM813_FAULT_EVENT_REG2, val);
    snprintf(buf, sizeof(buf), "%d", val);
    asr_property_set("bl2.pdreason2", buf);
    break;

  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
static unsigned restart_cmd_reg, restart_status_reg;

/*
 * PM802/PM803: base reg 0xe8
 * PM813/PM813s: base reg 0xe9
 * PM802S: base reg 0xcf
 */
void
pmic_init(void)
{
  uint8_t pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  if(pi2c_init(PMIC_BASE_PAGE_ADDR) < 0) {
    goto error;
  }

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    goto error;
  }

  switch(PMIC_PID(pmic_id)) {
  case PM802:
  case PM803:
    restart_cmd_reg = PM802_RTC_SPARE_REG1;
    restart_status_reg = PM802_RTC_SPARE_REG1;
    break;
  case PM802S:
  case PM803L:
    restart_cmd_reg = PM802S_RTC_SPARE_REG1;
    restart_status_reg = PM802S_RTC_SPARE_REG1;
    break;
  case PM816:
    restart_cmd_reg = PM816_RTC_SPARE_REG1;
    restart_status_reg = PM816_RTC_SPARE_REG1;
    break;
  case PM813L:
    restart_cmd_reg = PM813L_RTC_SPARE_REG1;
    restart_status_reg = PM813L_RTC_SPARE_REG1;
    break;
  case PM807:
    restart_cmd_reg = PM807_RTC_SPARE_REG1;
    restart_status_reg = PM807_RTC_SPARE_REG1;
  break;
  default:
    restart_cmd_reg = PM813_RTC_SPARE_REG1;
    restart_status_reg = PM813_RTC_SPARE_REG1;
    break;
  }

  return;
error:
  LOG_ERR("pmic_init failed\n");
  return;
}
/*---------------------------------------------------------------------------*/
int
pmic_set_restart_status(char reason)
{
  uint8_t reg_val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return 0;
  }

  pi2c_init(PMIC_BASE_PAGE_ADDR);
  pi2c_write_reg(restart_status_reg, reason);
  pi2c_read_reg(restart_status_reg, &reg_val);
  LOG_INFO("Set restart status = 0x%x\n", reg_val);

  return 0;
}
/*---------------------------------------------------------------------------*/
void
pmic_get_restart_cmd(char *r_cmd)
{
  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    *r_cmd = RESTART_CMD_NORMAL_POWERON;
    LOG_INFO("FPGA: Get restart cmd = 0x%x\n", (unsigned)*r_cmd);
    return;
  }

  pi2c_init(PMIC_BASE_PAGE_ADDR);
  pi2c_read_reg(restart_cmd_reg, (unsigned char *)r_cmd);
  LOG_INFO("Get restart cmd = 0x%x\n", (unsigned)*r_cmd);
}
/*---------------------------------------------------------------------------*/
int
pmic_set_restart_cmd(char cmd)
{
  uint8_t reg_val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return 0;
  }

  pi2c_init(PMIC_BASE_PAGE_ADDR);
  pi2c_write_reg(restart_cmd_reg, cmd);
  pi2c_read_reg(restart_cmd_reg, &reg_val);
  LOG_INFO("Set restart cmd = 0x%x\n", reg_val);

  return 0;
}
/*---------------------------------------------------------------------------*/
/* since pm802/802s/pm803 no charger module pmic, so isSysRestartByCharging() will never be "C"(sdk checks power_up_reason register, if charging, set RTC to 'C'),
 * so always no usb charging case. If want to support it, need use pmic_set_user_flag().
 * only for pm803 craneG:
 * If burn complete with 'reboot', Hal_Powerup_Get_Reason() is "N", set pmic_get_user_flag is 0xF in flasher, do normal start
 * If burn without reboot, Hal_Powerup_Get_Reason() is "N", pmic_get_user_flag is not 0xF, do charging start.
 *
 * PM813/PM813s: base reg 0xe8 low 4bits
 * PM802/PM802s/PM803: base reg 0xec high 4bits
 */
int
pmic_set_user_flag(char cmd)
{
  uint8_t reg_val, pmic_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return 0;
  }

  pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return -1;
  }

  if((cmd == RESTART_CMD_NORMAL_POWERON) || (cmd == RESTART_CMD_FORCE_NORMAL_POWERON)) {
    pi2c_init(PMIC_BASE_PAGE_ADDR);

    switch(PMIC_PID(pmic_id)) {
    /*case PM802: / * craneM * / */
    /*case PM802S: / * craneM * / */
    case PM803: /* craneG */
      pi2c_read_reg(PM802_REG_FOR_USR_DEF_FLAGS_STR, &reg_val);
      reg_val &= ~0xF0;
      reg_val |= UI_FLAG_REBOOT << 4;
      pi2c_write_reg(PM802_REG_FOR_USR_DEF_FLAGS_STR, reg_val);
      break;
    default:
      break;
    }
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
pmic_sw_reboot(void)
{
  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  /* each bit should be set separately */
  pmic_fault_wu_en_enable();
  pmic_set_fault_wu();
  pmic_set_sw_powerdown();
}
/*---------------------------------------------------------------------------*/
void
pmic_sw_pdown(void)
{
  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }
//QUECTEL ADD  CASE FAE-151256
//ASR source code issue, this function needs to be blocked
 // sys_close_all();

  /* each bit should be set separately */
  pmic_fault_wu_en_disable();
  pmic_set_sw_powerdown();
}
/*---------------------------------------------------------------------------*/
/*
 * crane:        28LP,     vol=1.05v
 * craneG Z2/A0: 22nm ULP, vol=0.8v
 * craneM:       22nm ULP, vol=0.9v
 *
 * crane:
 * defaultly vol = 1.05v
 * no matter cpu freq is 416M or 624M, 1.05v can wrok normally.
 * When cpu freq is 624M, cpu vol can not ve 1.0v
 * if cp load, cpu core voltage: case1: 0xA0, 1.0v, 416M; case2: 0xA4, 1.05v, 624M
 *
 * power regs: 0x20 & 0x21:
 * pm813/pmic813s/pm802/pm803: same
 */
void
pmic_set_buck1_vol(unsigned int chip_id)
{
  uint8_t reg;
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  uint8_t pmic_pid = PMIC_PID(pmic_id);
  if(PM816 == pmic_pid) {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM816_DVC_SET_REG, &reg);
    pi2c_write_reg(PM816_DVC_SET_REG, reg & ~DVC_ENALE);      /*Bit7 = 0, disable dvc */
    pi2c_read_reg(PM816_DVC_SET_REG, &reg);
    LOG_INFO("Set PM816_DVC_SET_REG = 0x%x, set bit7 to 0.\n", reg);
    /* default is 0x20, represents 0.8v, Cr5 need to set buck1 0.975v, weihai only has 0.97v/0.98v, so use 0.98v */
    /* 0.98V in order to prevent the problem the low temperature cann't boot(CraneLH 1606s WB8M DRO min) */
    /* the value will be set back to 0.9V in the cp stage */
    val = (chip_id == CHIP_ID_CRANEW) ? 0x32 : 0x20; /* 0.98v for CraneW and 0.8V for Fulmar*/
    pi2c_write_reg(PM816_BUCK1_VOLTAGE_SET_REG, val);
    pi2c_read_reg(PM816_BUCK1_VOLTAGE_SET_REG, &reg);
    LOG_INFO("Set cpu core voltage, PM816_BUCK1_VOLTAGE_SET_REG = 0x%x\n", reg);
  } else if((PM803L == pmic_pid)||(PM807 == pmic_pid)) { /* PM803L/PM807 doesn't support DVC */
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_write_reg(PM813_BUCK1_VOLTAGE_SET_REG, 0xB2); /*0.98V */
    pi2c_read_reg(PM813_BUCK1_VOLTAGE_SET_REG, &reg);
    LOG_INFO("Set cpu core voltage, PM803L/807_BUCK1_VOLTAGE_SET_REG = 0x%x\n", reg);
  } else if(PM813L == pmic_pid) {
      pi2c_init(PMIC_POWER_PAGE_ADDR);
      pi2c_read_reg(PM813L_DVC_SET_REG, &reg);
      pi2c_write_reg(PM813L_DVC_SET_REG, reg & ~DVC_ENALE);      /*Bit7 = 0, disable dvc */
      pi2c_read_reg(PM813L_DVC_SET_REG, &reg);
      LOG_INFO("Set PM813L_DVC_SET_REG = 0x%x, set bit7 to 0.\n", reg);
      val = 0x1e; /* 0.975v for CraneLS/CraneGT/CraneLB */
      pi2c_write_reg(PM813L_BUCK1_VOLTAGE_SET_REG, val);
      pi2c_read_reg(PM813L_BUCK1_VOLTAGE_SET_REG, &reg);
      LOG_INFO("Set cpu core voltage, PM813L_BUCK1_VOLTAGE_SET_REG = 0x%x\n", reg);
  } else {
    /* when set cpu core vol, must disable dvc firstly, defaultly vol = 1.05v */
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM813_DVC_SET_REG, &reg);
    pi2c_write_reg(PM813_DVC_SET_REG, reg & ~DVC_ENALE);      /*Bit7 = 0, disable dvc */
    pi2c_read_reg(PM813_DVC_SET_REG, &reg);
    LOG_INFO("Set PM813_DVC_SET_REG = 0x%x, set bit7 to 0.\n", reg);

    switch(chip_id) {
    case CHIP_ID_CRANE:
      val = 0xA4;     /* 1.05v */
      break;
    case CHIP_ID_CRANEG:
    case CHIP_ID_CRANEM:
    case CHIP_ID_CRANEL:
    case CHIP_ID_CRANELS:
    case CHIP_ID_CRANELR:
    case CHIP_ID_CRANELG:
    case CHIP_ID_CRANELRH:
    default:
      val = 0x9e;     /* 0.975V in order to prevent the problem the low temperature cann't boot */
      break;          /* the value will be set back to 0.9V in the cp stage */
    }

    pi2c_write_reg(PM813_BUCK1_VOLTAGE_SET_REG, val);
    pi2c_read_reg(PM813_BUCK1_VOLTAGE_SET_REG, &reg);
    LOG_INFO("Set cpu core voltage, PM813_BUCK1_VOLTAGE_SET_REG = 0x%x\n", reg);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * Set PMIC to skip mode
 *
 * pm813/pmic813s/pm802/pm802s/pm803/pm803l power regs: 0x22[5]
 * pm816 power regs: 0x36[4]
 */
void
pmic_set_buck1_skipmode(unsigned int chip_id)
{
  uint8_t reg;
  (void)chip_id;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  uint8_t pmic_pid = PMIC_PID(pmic_id);

  if(PM816 == pmic_pid) {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM816_FSM_BUCK1_REG2, &reg);
    pi2c_write_reg(PM816_FSM_BUCK1_REG2, reg | PM816_FSM_BUCK1_GO2PWM);        /*Bit4 = 1, go to pwm */
    pi2c_read_reg(PM816_FSM_BUCK1_REG2, &reg);
    LOG_INFO("Set skip mode PM816_FSM_BUCK1_REG2 = 0x%x, set bit4 to 1.\n", reg);
  } else if(PM803L == pmic_pid) {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM803L_FSM_BUCK1_REG5, &reg);
    pi2c_write_reg(PM803L_FSM_BUCK1_REG5, reg & ~PM803L_FSM_BUCK1_GO2PWM);   /*Bit0 = 0, go to pwm */
    pi2c_read_reg(PM803L_FSM_BUCK1_REG5, &reg);
    LOG_INFO("Set skip mode PM803L_FSM_BUCK1_REG5 = 0x%x, set bit0 to 0.\n", reg);
  } else if(PM807 == pmic_pid) {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM807_FSM_BUCK1_REG3, &reg);
    pi2c_write_reg(PM807_FSM_BUCK1_REG3, reg | PM807_FSM_BUCK1_GO2PWM);    /*Bit0 = 1, go to pwm */
    pi2c_read_reg(PM807_FSM_BUCK1_REG3, &reg);
    LOG_INFO("Set skip mode PM807_FSM_BUCK1_REG3 = 0x%x, set bit0 to 1.\n", reg);
  } else if(PM813L == pmic_pid) {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM813L_FSM_BUCK1_REG2, &reg);
    pi2c_write_reg(PM813L_FSM_BUCK1_REG2, reg | PM813L_FSM_BUCK1_GO2PWM);   /*Bit0 = 1, go to pwm */
    pi2c_read_reg(PM813L_FSM_BUCK1_REG2, &reg);
    LOG_INFO("Set skip mode PM813L_FSM_BUCK1_REG2 = 0x%x, set bit0 to 1.\n", reg);
  } else {
    pi2c_init(PMIC_POWER_PAGE_ADDR);
    pi2c_read_reg(PM803_FSM_BUCK1_REG2, &reg);
    pi2c_write_reg(PM803_FSM_BUCK1_REG2, reg | PM803_FSM_BUCK1_GO2PWM);        /*Bit5 = 1, go to pwm */
    pi2c_read_reg(PM803_FSM_BUCK1_REG2, &reg);
    LOG_INFO("Set skip mode PM_FSM_BUCK1_REG2 = 0x%x, set bit5 to 1.\n", reg);
  }
}
/*---------------------------------------------------------------------------*/
/*
 * PM802/PM802S/PM803: empty
 * PMIC813/PMIC813s: power reg 0x8F,
 * default val = 0xe(ldo11_disable, 1.8v), update to 0x42(ldo11_enabe, 1.2v)
 * Bit 7       6            5           4         3                  2                    1           0
 *           | ldo11_en |           ldo11_vset[3:0]                                |      dischg_en      | pd_ilim
 */
static void
pmic_set_ldo11_en_1_2v_for_PMIC813(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  pi2c_init(PMIC_POWER_PAGE_ADDR);

  pi2c_read_reg(PM813_LDO11_REG, &val);
  val |= LDO11_ENABLE;
  val = (val & ~LDO11_VSET_1_2V_BIT) | LDO11_VSET_1_2V;
  pi2c_write_reg(PM813_LDO11_REG, val);
  pi2c_read_reg(PM813_LDO11_REG, &val);
  LOG_INFO("enable ldo11 1.2v: [0x%x] = 0x%x\n", PM813_LDO11_REG, val);
}
void
pmic_set_uhs_voltage(void)
{
  pmic_set_ldo11_en_1_2v_for_PMIC813();
}
/* In order to meet the power-on sequence of CraneGT DDR Phy requirements,
   need to set 1.2V voltage within 20ms after power-on. Therefore, set LDO11 to 1.2V in the preboot stage */
void
pmic_set_ddr_phy_voltage(void)
{
  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  if(PM813S != PMIC_PID(pmic_id)) {
    return;
  }

  pmic_set_ldo11_en_1_2v_for_PMIC813();
}
/*---------------------------------------------------------------------------*/
/*
 * PM803: power reg 0x82 /0x83,
 * default val = 0xe(ldo7, 1.2v), update to (ldo7, 1.6v)
 */
void
pmic_set_ldo7_en_1_6v_for_craneM_and_pm803(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  if(PM803 != PMIC_PID(pmic_id)) {
    return;
  }

  pi2c_init(PMIC_POWER_PAGE_ADDR);

  pi2c_read_reg(PM803_LDO7_SLEEP_MODE_REG, &val);
  val = (val & ~LDO7_VSET_1_6V_BIT) | LDO7_VSET_1_6V;
  pi2c_write_reg(PM803_LDO7_SLEEP_MODE_REG, val);
  pi2c_read_reg(PM803_LDO7_SLEEP_MODE_REG, &val);
  LOG_INFO("CraneM pm803: set ldo7 sleep mode 1.6v: [0x%x] = 0x%x\n", PM803_LDO7_SLEEP_MODE_REG, val);

  pi2c_read_reg(PM803_LDO7_ACTIVE_MODE_REG, &val);
  val = (val & ~LDO7_VSET_1_6V_BIT) | LDO7_VSET_1_6V;
  pi2c_write_reg(PM803_LDO7_ACTIVE_MODE_REG, val);
  pi2c_read_reg(PM803_LDO7_ACTIVE_MODE_REG, &val);
  LOG_INFO("CraneM pm803: set ldo7 active mode 1.6v: [0x%x] = 0x%x\n", PM803_LDO7_ACTIVE_MODE_REG, val);
}

/*
 * PM807: power reg 0x7A,
 * default val = 0x8e(ldo4, 3.1v), update to (ldo4, 3.0v)
 */
#ifdef QUECTEL_PREBOOT_OPT 
void
pmic_set_ldo4_en_3v_for_craneLG_LRH_and_pm807(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  if(PM807 != PMIC_PID(pmic_id)) {
    return;
  }

  pi2c_init(PMIC_POWER_PAGE_ADDR);

  pi2c_read_reg(PM807_LDO4_ACTIVE_MODE_REG, &val);
  val = (val & ~PM807_LDO4_VSET_3V_BIT) | PM807_LDO4_VSET_3V;
  pi2c_write_reg(PM807_LDO4_ACTIVE_MODE_REG, val);
  pi2c_read_reg(PM807_LDO4_ACTIVE_MODE_REG, &val);
  LOG_INFO("CraneLG pm807: set ldo4 output setting under active mode 3v: [0x%x] = 0x%x\n", 0x7A, val);
}
#endif
/*---------------------------------------------------------------------------*/
/*
 * LDO2 supply power for fuse module (pad: QVPS pin)
 * only CraneL pm803 need 1.9v when burn fuse
 * PM803: power reg 0x74,
 * default val = 0x4(ldo2, 1.8v), update to 0x5 (ldo2, 1.9v)
 */
static void
_pmic_set_fuse_voltage(unsigned int vol)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  if(PM803 != PMIC_PID(pmic_id)) {
    return;
  }

  if(CHIP_ID_CRANEL != hw_chip_id() &&
     CHIP_ID_CRANELS != hw_chip_id() &&
     CHIP_ID_CRANELG != hw_chip_id() &&
     CHIP_ID_CRANELRH != hw_chip_id() &&
     CHIP_ID_CRANELR != hw_chip_id()) {
    return;
  }

  pi2c_init(PMIC_POWER_PAGE_ADDR);
  pi2c_read_reg(0x74, &val);
  LOG_INFO("CraneL pm803: read ldo2 BIT 3:0 = 0x%x\n", val & 0xF);

  val = (val & ~0xF) | vol;
  pi2c_write_reg(0x74, val);
  pi2c_read_reg(0x74, &val);
  LOG_INFO("CraneL pm803: set ldo2 vol to 0x%x , then read BIT 3:0 = 0x%x\n", vol, val & 0xF);
}
#ifdef BOARD_CRANEW_MCU
#define PM813S_LDO_1_2_V  0x42
#define PM813S_LDO_1_8_V  0x4E
#define PM813S_LDO_2_8_V  0x6A
#define PM813S_LDO_3_3_V  0x7E

#define PM813S_POWER_LDO1 0x71
#define PM813S_POWER_LDO2 0x74
#define PM813S_POWER_LDO4 0x7A
#define PM813S_POWER_LDO5 0x7D
#define PM813S_POWER_LDO6 0x80
#define PM813S_POWER_LDO7 0x83
#define PM813S_POWER_LDO9 0x89
#define PM813S_POWER_LDO11 0x8F
#define PM813S_POWER_LDO12 0x92

#define PM816_LDO_1_8_V   0x06
#define PM816_LDO_3_3_V   0x0F

#define PM816_POWER_LDO11 0xCF
#define PM816_POWER_LDO12 0xD2
#define PM816_POWER_LDO11_EN 0x12
#define PM816_POWER_LDO12_EN 0x13

#define PM816_POWER_BUCKLDO_CTRL3 0x10

/*
 ***************************************************************************************
    //ldo1, rf, default is off
    pi2c_write_reg(PM813S_POWER_LDO1, PM813S_LDO_2_8_V);
    pi2c_read_reg(PM813S_POWER_LDO1, &val);
    LOG_INFO("Cranew: set ldo1 active mode 2.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO1, val);

    //ldo2, usb, default is on
    pi2c_write_reg(PM813S_POWER_LDO2, PM813S_LDO_3_3_V);
    pi2c_read_reg(PM813S_POWER_LDO2, &val);
    LOG_INFO("Cranew: set ldo2 active mode 3.3v: [0x%x] = 0x%x\n", PM813S_POWER_LDO2, val);

    //ldo5, TCXO_SW
    pi2c_write_reg(PM813S_POWER_LDO5, PM813S_LDO_1_8_V);
    pi2c_read_reg(PM813S_POWER_LDO5, &val);
    LOG_INFO("Cranew: set ldo5 active mode 1.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO5, val);

    //ldo6, AVDD18_AP, default is on
    pi2c_write_reg(PM813S_POWER_LDO6, PM813S_LDO_1_8_V);
    pi2c_read_reg(PM813S_POWER_LDO6, &val);
    LOG_INFO("Cranew: set ldo6 active mode 1.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO6, val);

    //ldo7, RF_1P8V, default is on
    pi2c_write_reg(PM813S_POWER_LDO7, PM813S_LDO_1_8_V);
    pi2c_read_reg(PM813S_POWER_LDO7, &val);
    LOG_INFO("Cranew: set ldo7 active mode 1.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO7, val);

    //ldo11, VRF_1P2V_SW, default is off
    pi2c_write_reg(PM813S_POWER_LDO11, PM813S_LDO_1_2_V);
    pi2c_read_reg(PM813S_POWER_LDO11, &val);
    LOG_INFO("Cranew: set ldo11 active mode 1.2v: [0x%x] = 0x%x\n", PM813S_POWER_LDO11, val);

    //ldo12, AVDD peripheral, default is off
    pi2c_write_reg(PM813S_POWER_LDO12, PM813S_LDO_2_8_V);
    pi2c_read_reg(PM813S_POWER_LDO12, &val);
    LOG_INFO("Cranew: set ldo12 active mode 2.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO12, val);
 ***************************************************************************************
 * emmc, default ldo4 and ldo9 is off, need on. if ldo off,  emmc cmd1 will no response.
 */
void
pmic_LDO_config_for_cranew_mcu_and_pm813s(void)
{
  unsigned char val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  uint8_t pmic_id = pmic_get_id();
  if(pmic_id == PMIC_UNAVAILABLE) {
    return;
  }

  if(PM813S == PMIC_PID(pmic_id)) {
    /*ldo4, VDD emmc, default is off, active it in preboot. */
    pi2c_write_reg(PM813S_POWER_LDO4, PM813S_LDO_3_3_V);
    pi2c_read_reg(PM813S_POWER_LDO4, &val);
    LOG_INFO("Cranew: set ldo4 active mode 3.3v: [0x%x] = 0x%x\n", PM813S_POWER_LDO4, val);

    /*ldo9, VIO emmc, default is off, active it in preboot. */
    pi2c_write_reg(PM813S_POWER_LDO9, PM813S_LDO_1_8_V);
    pi2c_read_reg(PM813S_POWER_LDO9, &val);
    LOG_INFO("Cranew: set ldo9 active mode 1.8v: [0x%x] = 0x%x\n", PM813S_POWER_LDO9, val);
  }
#if  0
  /* weihai B1, BT/BLE 1.2v AVDD and PA is off by default, for BT test, temperally enable buck4_sw4 */
  else if(PM816 == PMIC_PID(pmic_id)) {
    pi2c_read_reg(PM816_POWER_BUCKLDO_CTRL3, &val);
    pi2c_write_reg(PM816_POWER_BUCKLDO_CTRL3, (val | (1 << 6)));   /*enable buck4_sw4 for BT */
    pi2c_read_reg(PM816_POWER_BUCKLDO_CTRL3, &val);
    LOG_INFO("Fulmar: set buck4_sw4 on: [0x%x] = 0x%x\n", PM816_POWER_BUCKLDO_CTRL3, val);
  }
#endif
#if 0  /*for PM816 , ldo11 and ldo12, already enbaled in OTP, don't need set again */
  else { /*PM816 */
    /*ldo12, VDD emmc, active it in preboot if it's off by default. */
    pi2c_write_reg(PM816_POWER_LDO12, PM816_LDO_3_3_V);
    pi2c_read_reg(PM816_POWER_LDO12, &val);
    LOG_INFO("Cranew: set ldo12 active mode 3.3v: [0x%x] = 0x%x\n", PM816_POWER_LDO12, val);

    /*ldo11, VIO emmc, active it in preboot if it's off by default. */
    pi2c_write_reg(PM816_POWER_LDO11, PM816_LDO_1_8_V);
    pi2c_read_reg(PM816_POWER_LDO11, &val);
    LOG_INFO("Cranew: set ldo11 active mode 1.8v: [0x%x] = 0x%x\n", PM816_POWER_LDO11, val);

    pi2c_read_reg(PM816_POWER_LDO12_EN, &val);
    pi2c_write_reg(PM816_POWER_LDO12_EN, (val | (1 << 0))); /*enable LDO12 */
    pi2c_read_reg(PM816_POWER_LDO11_EN, &val);
    pi2c_write_reg(PM816_POWER_LDO11_EN, (val | (1 << 7))); /*enable LDO11 */
  }
#endif
  return;
}
#endif
/*---------------------------------------------------------------------------*/
void
pmic_set_fuse_voltage(void)
{
  _pmic_set_fuse_voltage(PM803_LDO_2_VOL_1_9V);
}
/*---------------------------------------------------------------------------*/
void
pmic_restore_fuse_voltage(void)
{
  _pmic_set_fuse_voltage(PM803_LDO_2_VOL_1_8V);
}
#endif
