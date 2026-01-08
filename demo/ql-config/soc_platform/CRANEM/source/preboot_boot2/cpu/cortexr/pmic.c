/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */
#include <stdio.h>
#include <stdint.h>
#include "pmic.h"
#include "reg.h"
#include "i2c.h"
#include "cpu.h"
#include "smux.h"
/* Log configuration */
#include "log.h"
#define LOG_MODULE "PMIC"
#define LOG_LEVEL  LOG_LEVEL_MAIN

//quectel add 
#define  QUECTEL_PREBOOT_OPT

#ifdef  QUECTEL_PREBOOT_OPT
//************ PM812 ******************
#define PM812_STATUS_1_REG	0x01
#define PM812_ONKEY_STATUS_BIT	(0x01<<0)
#define PM812_EXTON_STATUS_BIT	(0x01<<1)
#define PM812_VCHG_STATUS_BIT	(0x01<<2)
#define PM812_VBAT_STATUS_BIT	(0x01<<3)
#define PM812_VBUS_STATUS_BIT	(0x01<<4)
#define PM812_STATUS_2_REG	0x02
#define PM812_RTC_ALARM_STATUS_BIT	(0x01<<0)
#define PM812_INT_STATUS_1_REG	0x05
#define PM812_INT_STATUS_2_REG	0x06
#define PM812_INT_STATUS_3_REG	0x07
#define PM812_INT_STATUS_4_REG	0x08
#define PM812_INT_ENABLE_REG_1	0x09
#define PM812_INT_ENABLE_REG_2	0x0a
#define PM812_INT_ENABLE_REG_3	0x0b
#define PM812_INT_ENABLE_REG_4	0x0c
#define PM812_WAKEUP_1_REG	0x0d
#define PM812_RESET_PMIC_BIT (0x01<<6)
#define PM812_PWM_REG1          0x40
#define PM812_PWM_REG4          0x43
#define PM812_WD_MODE_BIT		(0x01<<0)
#define PM812_WD_RST_BIT		(0x01<<4)
#define PM812_SW_PDOWN_BIT	(0x01<<5)
#define PM812_WAKEUP_2_REG	0x0e
#define PM812_RTC_MISC_5_REG 0xe7
#define PM812_RTC_FAULT_WU_EN_BIT (0x01<<2)

unsigned char power_up_flag = 0xff;
#endif

static uint8_t g_pmic_id = PMIC_UNAVAILABLE;
/*---------------------------------------------------------------------------*/
/*
 * default value 75mA, set to 150mA.
 *
 * base regs: 0x21:
 * pm813/pmic813s: valid
 * pm802/pm803   : reserved, blank.
 */
static void
pmic_pre_charge_current_setting(void)
{
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  LOG_INFO("Set prcharge current to 150mA\n");
  pi2c_init(PMIC_BASE_PAGE_ADDR);

  pi2c_read_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, &val);
  pi2c_write_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, (val & 0xF3) | PRE_CHARGE_CURRENT_SETTING_150MA);
  pi2c_read_reg(PM813_PRE_CHARGE_CURRENT_SETTING_REG, &val);
  LOG_INFO("PM813_PRE_CHARGE_CURRENT_SETTING_REG: [0x%x] = 0x%x\n", PM813_PRE_CHARGE_CURRENT_SETTING_REG, val);
}
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

#ifdef  QUECTEL_PREBOOT_OPT //Ramos.zhang/20200323, Quectel add for  powerkey  power on
extern void clock_wait_ms(unsigned ms);
void quec_pmic_sw_pdown(void)
{
   uint8_t reg_tmp;
   clock_wait_ms(10); 
   pi2c_init(PMIC_BASE_PAGE_ADDR);

   // PMIC faul disable
   pi2c_read_reg(PM812_RTC_MISC_5_REG, &reg_tmp);
   reg_tmp &= ~(PM812_RTC_FAULT_WU_EN_BIT);
   pi2c_write_reg(PM812_RTC_MISC_5_REG,reg_tmp);

   // PMIC down
   pi2c_read_reg(PM812_WAKEUP_1_REG, &reg_tmp);
   reg_tmp |= PM812_SW_PDOWN_BIT;
   pi2c_write_reg(PM812_WAKEUP_1_REG,reg_tmp);
}

int Quec_PMIC_GET_PWEKY_STATUS(void)
{
	uint8_t reg_val;
	pi2c_read_reg(PM812_STATUS_1_REG, &reg_val);
	//printf("powerkey STATUS_1_REG=0x%x!!!!\r\n",reg_val);
	if(reg_val & PM812_EXTON_STATUS_BIT)
	{
		//printf("powerkey down low level !!!!\r\n");
		return 1;
	}
	else
	{
		//printf("powerkey high level\r\n");
		return 0;
	}
	//printf("powerkey on know level\r\n");
	return 0;
}


int Quec_PowerOnCheck(void)
{
    unsigned int i=0;

    //uart_printf("[Quec][PWK]OnkeyPowerOnCheck\r\n");
    /* Set 15s for power-down-by-long-ONKEY-pressing period, this register is 
    restricted to change only one bit each time. */
    pi2c_init(PMIC_BASE_PAGE_ADDR);

    for(i=0;i<5;i++) // 50ms+270ms(run here need 270ms )= 320ms for powerkey check.
    {
        if (!Quec_PMIC_GET_PWEKY_STATUS())
        {
            LOG_PRINT("short press, do poweroff!!\r\n");
	    	clock_wait_ms(10); /* 100 ms */
            return 1; //Power down if ONKEY is not pressed long enough.
        }
        clock_wait_ms(5); /* 5 ms */
    }
	LOG_PRINT("long press, do poweron!!\r\n");
    return 0;
}

void read_update_pmic_reg(void)
{
	uint8_t pmic_reg[8] = {0};
	uint8_t value_13=0,value_14=0;

	printf("\r\nPMIC REG:0x00,0x01,0x10,0x13,0x14,0x18,0xE2,0xE5,0xE6,0xF0\r\n");
	
	pi2c_read_reg(0x00,&pmic_reg[0]);
	pi2c_read_reg(0x01,&pmic_reg[1]);
	pi2c_read_reg(0x10,&pmic_reg[2]);
	
	pi2c_read_reg(0x13,&value_13);
	pi2c_read_reg(0x14,&value_14);
	
	pi2c_read_reg(0x18,&pmic_reg[3]);
	pi2c_read_reg(0xE2,&pmic_reg[4]);
	pi2c_read_reg(0xE5,&pmic_reg[5]);
	
//	arom_pi2c_write_reg(0xE2, (pmic_reg[4]&0xF0)|0xF1);
	
	pi2c_read_reg(0xE6,&pmic_reg[6]);
	pi2c_read_reg(0xF0,&pmic_reg[7]);
	
	printf("PMIC VAL:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n", pmic_reg[0], pmic_reg[1], pmic_reg[2], value_13, value_14, pmic_reg[3], pmic_reg[4], pmic_reg[5], pmic_reg[6], pmic_reg[7]);

	pi2c_write_reg(0xE5, pmic_reg[5]);
	pi2c_write_reg(0xE6, pmic_reg[6]);

#if 0  //add workaround for unexpected EXTON2 wakup
	if (((pmic_reg[1] & GUILIN_EXTON1_STATUS) != GUILIN_EXTON1_STATUS) && ((pmic_reg[2] & GUILIN_EXTON2_WAKEUP) == GUILIN_EXTON2_WAKEUP) )
	{
		printf("+exton2 wakeup only,shutdown\r\n");
		arom_pi2c_write_reg(0x13, 0x02);
		arom_pi2c_write_reg(0x14, 0x7c);
		printf("PMIC REG(0x13,0x14) :0x%x,0x%x\r\n",GuilinBaseRead(0x13),GuilinBaseRead(0x14));
		Delay(100*1000); //add 100ms delay
		System_poweroff();
	}
#endif
}

#endif
/*---------------------------------------------------------------------------*/
/*
 * preboot set 10pf
 * cp will set 20pf after 1s
 * pm803 do not have CRYSTAL select bits
 */
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
#ifdef  QUECTEL_PREBOOT_OPT
  unsigned char reg[6];
#endif

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  while(pmic_reset_retry--) {
    pi2c_bus_reset();
    pi2c_reset();
    pi2c_init(PMIC_BASE_PAGE_ADDR);
    if(pi2c_read_reg(PMIC_ID, &val) == 0) {
      break;
    }
  }

#if 0
 if (((val >> 4) & 0xF) == PM802)
 {
  pi2c_read_reg(0x10, &reg[0]); //read power up log
  pi2c_read_reg(0xf0, &reg[1]); //read xo reg
  pi2c_read_reg(0x01, &reg[2]); //read exton1n status
  
  if((reg[1] & 0xf0) == 0x0) //cold power up
  {
   if((reg[0] == 0x0) && ((reg[2] & 0x02) == 0)) // unkown wakeup source && pwrkey is not pressed
   {
    printf("unkown wakeup:%d, do power off!!!\n",reg[0]);

    pi2c_write_reg(0xe2, 0xf0); //set discharger timer to 0S //default 2S
    pi2c_read_reg(0xe7, &reg[0]);
    pi2c_write_reg(0xe7, (reg[0] & 0xfb)); //disalbe fault wakeup
    pi2c_read_reg(0x0d, &reg[0]);
    pi2c_write_reg(0x0d, (reg[0])|0x20) ;
    return;
   }
  }
  pi2c_write_reg(0xe2, 0xf1); //set discharger timer  back to 1S
}
#endif

#ifdef  QUECTEL_PREBOOT_OPT
	#if 0
    pi2c_read_reg(0x01, &reg[0]);
    pi2c_read_reg(0x10, &reg[1]);
    pi2c_read_reg(0xe5, &reg[2]);
    pi2c_read_reg(0xe6, &reg[3]);
    pi2c_read_reg(0xf0, &reg[4]);
	#endif
    pi2c_read_reg(0xeb, &reg[5]);
	if(smux_running_mode_get() == SMUX_MODE_NORMAL)
	{
		power_up_flag = 0x01; // hot  power on
		LOG_PRINT("download mode, do power on\n"); 
	}
	else
	{
		 if( 0 == (reg[5]&0x80))
		{
			power_up_flag = 0x00; // cold power on
			LOG_PRINT("cold power on\n"); 
		}
		else
		{
			power_up_flag = 0x01; // hot  power on
			LOG_PRINT("hot power on\n"); 
		}	
		if(0x00 == power_up_flag) // cold power on
	    {
        	if(Quec_PowerOnCheck())
		    {
			    LOG_PRINT("warning: powkey too short !!!!!\r\n");
                //quec_pmic_sw_pdown();
				pmic_sw_pdown();
                return;
		    }
			//save hot flag
			pi2c_write_reg(0xeb, (reg[5] | 0x80));
        }
	}
	read_update_pmic_reg();
#endif

  if(pi2c_init(PMIC_BASE_PAGE_ADDR) < 0) {
    goto error;
  }

  val = pmic_get_id();
  if(val == PMIC_UNAVAILABLE) {
    goto error;
  }

  LOG_PRINT("Found PMIC with Id: 0x%x\n", (unsigned)val);
  pmic_set_crystal();
  pmic_pre_charge_current_setting(); //only pm813 support charger

  LOG_INFO("Disabling PMIC watchdog\n");     /* work as a delay */

  /* disable watchdog */
  if(pi2c_write_reg(PMIC_WATCHDOG, WATCHDOG_WD_DIS) < 0) {
    goto error;
  }

  val = 0;
  if(pi2c_read_reg(PMIC_WATCHDOG, &val) < 0) {
    goto error;
  }

  if(val & WATCHDOG_WD_DIS) {
    LOG_INFO("PMIC watchdog is disabled\n");
  } else {
    LOG_INFO("Disable watchdog failed, reg val = 0x%x\n", (unsigned)val);
  }

  return;
error:
  LOG_ERR("pmic_setup failed\n");
}
/*---------------------------------------------------------------------------*/
/*
 * base reg: 0x10
 * pm802/803/pm813/pm813s: valid
 * pm802/803:
 * bit 7       6             5               4                 3             2                 1                0
 *       |             |              | fault_wu_log     | bat_wu_log | rtc_alarm_wu_log | exton1n_wu_log | onkey_wu_log |
 * pm813/pm813s:
 *
 *       |ivbus_detect | fault_wu_log | rtc_alarm_wu_log | bat_wu_log | exton2_wu_log    | exton1n_wu_log | onkey_wu_log |
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
 * PM802
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
 * PM802
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
 * pm802
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
static unsigned restart_cmd_reg, restart_status_reg;

/*
 * PM802/PM803: base reg 0xe8
 * PM813/PM813s: base reg 0xe9
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
 * pm813/pmic813s: valid
 * pm802/pm803   : reserved, blank.
 */
void
pmic_set_buck_vol(unsigned int chip_id)
{
  uint8_t reg;
  uint8_t val;

  if(hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return;
  }

  /* when set cpu core vol, must disable dvc firstly, defaultly vol = 1.05v */
  pi2c_init(PMIC_POWER_PAGE_ADDR);
  pi2c_read_reg(PM813_DVC_SET_REG, &reg);
  pi2c_write_reg(PM813_DVC_SET_REG, reg & ~DVC_ENALE);    /*Bit7 = 0, disable dvc */
  pi2c_read_reg(PM813_DVC_SET_REG, &reg);
  LOG_INFO("Set PM813_DVC_SET_REG = 0x%x, set bit7 to 0.\n", reg);

  switch(chip_id) {
  case CHIP_ID_CRANE:
    val = 0xA4;   /* 1.05v */
    break;
  case CHIP_ID_CRANEG_Z2:
  case CHIP_ID_CRANEG:
    val = 0x90;   /* 0.8v */
    break;
  case CHIP_ID_CRANEM:
    val = 0x98;   /* 0.9v */
    break;
  default:
    val = 0x90;   /* 0.8v */
    break;
  }

  pi2c_write_reg(PM813_BUCK_VOLTAGE_SET_REG, val);
  pi2c_read_reg(PM813_BUCK_VOLTAGE_SET_REG, &reg);
  LOG_INFO("Set cpu core voltage, PM813_BUCK_VOLTAGE_SET_REG = 0x%x\n", reg);
}
/*---------------------------------------------------------------------------*/
/*
 * PM802/PM803: empty
 * PMIC813/PMIC813s: power reg 0x8F,
 * default val = 0xe(ldo11_disable, 1.8v), update to 0x42(ldo11_enabe, 1.2v)
 * Bit 7       6            5           4         3                  2                    1           0
 *           | ldo11_en |           ldo11_vset[3:0]                                |      dischg_en      | pd_ilim
 */
void
pmic_set_ldo11_en_1_2v_for_uhs(void)
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
  LOG_INFO("UHS enable ldo11 1.2v: [0x%x] = 0x%x\n", PM813_LDO11_REG, val);
}
/*---------------------------------------------------------------------------*/
