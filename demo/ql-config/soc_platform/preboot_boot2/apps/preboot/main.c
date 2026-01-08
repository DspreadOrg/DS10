#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "psram.h"
#include "pmic.h"
#include "cpu.h"
#include "secureboot.h"
#include "aboot.h"
#include "reg.h"
#include "smux.h"
#include "property.h"
#include "efuse.h"
#include "syscall-arom.h"
#include "pmu.h"
#if defined(CPU_MODEL_CRANE)
#include "freq.h"
#include "mpu.h"
#endif

#define NOT_MCU_CR5 ((AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5) && \
                     (AROM_VERSION != AROM_VER_2023_09_09_FULMAR_CR5))

#define SUPPORT_DDR_MEMORY (AROM_VERSION == AROM_VER_2022_11_11_CRANEG_CR5 || \
                            AROM_VERSION == AROM_VER_2023_09_09_FULMAR_CR5)

#define SUPPORT_PSRAM_MEMORY (AROM_VERSION != AROM_VER_2022_11_11_CRANEG_CR5 && \
                              AROM_VERSION != AROM_VER_2024_04_30_LAPWING_CR5 && \
                              AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5 && \
                              AROM_VERSION != AROM_VER_2023_09_09_FULMAR_CR5)
#if SUPPORT_DDR_MEMORY
#include "ddr.h"
#endif

#include "log.h"
#define LOG_MODULE "Preboot"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define SUPPORT_BOOT_GO_TO_PRODUCTION   0
/*---------------------------------------------------------------------------*/
#if defined(CPU_MODEL_CRANE)
extern char __lzma_decode_start, __lzma_decode_size;
#endif

/*---------------------------------------------------------------------------*/
#define UMTI_BASE                 0xD4207000
#define USB_PHY_REG_04            0x10
#define USB_PHY_REG_25            0x94
#define USB_PHY_REG_28            0xA0
#define USB_PHY_REG_29            0xA4
#define HS_ISEL_MASK              (0x3 << 12)
#define HS_ISEL                   (0x3 << 12)
/*---------------------------------------------------------------------------*/

#define	QUECTEL_PREBOOT_OPT
#ifdef QUECTEL_PREBOOT_OPT
#include "i2c.h"
static int Quec_PMIC_GET_PWEKY_STATUS(void)
{
	uint8_t reg_val;
	pi2c_read_reg(0x01, &reg_val);
	if(reg_val & (0x01<<1))
	{
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}
static int Quec_PowerOnCheck(void)
{
    unsigned int i=0;

    //uart_printf("[Quec][PWK]OnkeyPowerOnCheck\r\n");
    /* Set 15s for power-down-by-long-ONKEY-pressing period, this register is 
    restricted to change only one bit each time. */
    pi2c_init(PMIC_BASE_PAGE_ADDR);
	#if AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5
    for(i=0;i<80;i++) // 1602ƽ̨ 520ms+100ms(run here need 270ms )= 620ms for powerkey check.
    #else
	 for(i=0;i<5;i++) // 1606ƽ̨ 520ms+100ms(run here need 270ms )= 620ms for powerkey check.
	#endif
    {
        if (!Quec_PMIC_GET_PWEKY_STATUS())
        {
            LOG_PRINT("short press, do poweroff!!\r\n");
	    	clock_wait_ms(10); /* 100 ms */
            return 1; //Power down if ONKEY is not pressed long enough.
        }
        clock_wait_ms(1); /* 5 ms */
    }
	LOG_PRINT("long press, do poweron!!\r\n");
    return 0;
}
static void quec_powerkey_check(void)
{
	unsigned char power_up_flag = 0xff, i = 0;
	unsigned char val;
	int pmic_reset_retry = 3;
	unsigned char reg[10] = {0};

	while(pmic_reset_retry--) {
		pi2c_bus_reset();
		pi2c_reset();
		pi2c_init(PMIC_BASE_PAGE_ADDR);
		if(pi2c_read_reg(PMIC_ID, &val) == 0) {
			break;
		}
	}
	pi2c_read_reg(0x01, &reg[0]);//status 
	pi2c_read_reg(0x10, &reg[1]);
	pi2c_read_reg(0xe5, &reg[2]);
	pi2c_read_reg(0xe6, &reg[3]);
	pi2c_read_reg(0xe2, &reg[4]);
	pi2c_read_reg(0xeb, &reg[5]);
	LOG_PRINT("PMIC REG:0x01,0x10,0xe5,0xe6,0xe2,0xeb\r\n"); 
	LOG_PRINT("PMIC VAL:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x\r\n",reg[0],reg[1],reg[2],reg[3],reg[4],reg[5]);
	if(smux_running_mode_get() == SMUX_MODE_NORMAL)
	{
		power_up_flag = 0x01; // hot  power on
		LOG_PRINT("download mode, do power on\n"); 
	}
	else
	{
		if (PMIC_PID(val) == PM802)
		{
			pi2c_read_reg(0x10, &reg[0]); //read power up log
			pi2c_read_reg(0xf0, &reg[1]); //read xo reg
			pi2c_read_reg(0x01, &reg[2]); //read exton1n status

			if((reg[1] & 0xf0) == 0x0) //cold power up
			{
				if((reg[0] == 0x0) && ((reg[2] & 0x02) == 0)) // unkown wakeup source && pwrkey is not pressed
				{
					LOG_PRINT("unkown wakeup:%d, do power off!!!\n",reg[0]);

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
	for(i = 0;i < 10; i++)
	{
		if(reg[i] == 0x0)
		{
			reg[i] = 0xFF;
		}
	}
	reg[9] = 0x0;
	asr_property_set("power_down_reg", (const char*)(reg));
}
#endif
void
usb_phy_config(void)
{
  unsigned val;
  unsigned chip_id;

  chip_id = hw_chip_id();
  if(chip_id == CHIP_ID_CRANE) {
    /* driver strength */
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_28);
    writel((val & ~HS_ISEL_MASK) | HS_ISEL, UMTI_BASE + USB_PHY_REG_28);
    LOG_PRINT("Usb phy reg 28 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_28));
  } else if((CHIP_ID_CRANEG == chip_id) || (CHIP_ID_CRANEM == chip_id)) {
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_29);
    writel((val & ~0x1F) | 0x1B, UMTI_BASE + USB_PHY_REG_29);
    LOG_PRINT("Usb phy reg 29 0x%x -> 0x%lx\n", val, readl(UMTI_BASE + USB_PHY_REG_29));
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_25);
    writel(val | (BIT7), UMTI_BASE + USB_PHY_REG_25);
    val = (unsigned)readl(UMTI_BASE + USB_PHY_REG_04);
    writel(val | (BIT4 | BIT5 | BIT6), UMTI_BASE + USB_PHY_REG_04);
  }
}
/*---------------------------------------------------------------------------*/
#if defined(CPU_MODEL_CRANE) && (AROM_VERSION != AROM_VER_2024_04_30_LAPWING_CR5)
static void
psram_mpu_config(void)
{
#if SUPPORT_DDR_MEMORY
  extern char __ddr_base;
  unsigned base = (unsigned)&__ddr_base;
  unsigned size = get_ddr_size();
#else
  extern char __psram_base;
  unsigned base = (unsigned)&__psram_base;
  unsigned size = get_psram_size();
#endif

  mpu_region_config(ATCM_CODE_REGION_NUM, 0x0, 0x10000, MPU_ATTR_RWX);
  mpu_region_config(PSRAM_DDR_REGION_NUM_RWX, base, size, MPU_ATTR_RWX);
  mpu_region_config(PSRAM_DDR_REGION_NUM_NONE, 0x0, 0x80000000, MPU_ATTR_NONE);

/* These used memory(>= 0x8000_0000) should be configurated to MPU normal,
 * if not configurated in bootrom for Cortex R5.
 */
#if (AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0 || \
     AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5)
  extern char __aram_ctext_start, __aram_ctext_size;
  mpu_region_config(L2SRAM_REGION_NUM, &__aram_ctext_start, &__aram_ctext_size, MPU_ATTR_RWX);
#endif
}
#endif

#if NOT_MCU_CR5
static int
pmic_config(void)
{
  if (hw_platform_type() != HW_PLATFORM_TYPE_SILICON) {
    return 0;
  }

  pmic_setup();

  uint8_t powerup_reason = pmic_powerup_get_reason();
  LOG_PRINT("Power_up_reason=0x%x.\n", powerup_reason);

  pmic_fault_wu_en_disable();

  /* except bit6(ivbus_detect), bit1(exton1_wu_log) and bit0(onkey_wu_log), 0x67 = 0B 0100 0011 */
  if(!(powerup_reason & 0x67)) {
    LOG_WARN("Warning: power_up_reason=0x%x is abnormal.\n", powerup_reason);
    /* pmic_sw_pdown(); */
    /* return 0; */
  }

  switch(PMIC_PID(pmic_get_id())) {
  case PM802:
  case PM802S:
    /* workaround for PM802, if power up log = 0 or 8(bat_wu_log), pmic power down. PM813 no this hardware bug. */
    if(0 == powerup_reason || 8 == powerup_reason) {
      LOG_WARN("Warning: power_up_reason = 0 or 8(bat_wu_log). need pmic power down\n");
      pmic_sw_pdown();
      /* never reach here. */
    }
    break;
  case PM813L:
    /* pmic813L A0's bug: onkey reason cannot set bit in powerup_reason, So add workaround */
    break;
  default:
    break;
  }

  return 0;
}
#endif
/*---------------------------------------------------------------------------*/
int
main(void)
{
  /*read or write geu, must enable clk at first, else, geu bank dump always be 0. */
#if (AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5)
  /* watch1 cr5 does not require the use of the GEU module. */
  geu_clk_enable();
#endif

#if defined(CPU_MODEL_CRANE)
  cr5_axi_set_max_freq();
#endif

  LOG_PRINT("Executing preboot application...\n");
  LOG_PRINT("Preboot version: %s\n", BOOTLOADER_VERSION);

  /* force init prop 1K space: 0XB0020C00~ 0xB0021000 */
  asr_property_area_init(1);
  asr_property_set("bl2.preboot.ver", BOOTLOADER_VERSION);

#if defined(CPU_MODEL_CRANE) || defined(CPU_MODEL_CRANEW_MCU)
#if NOT_MCU_CR5
  /* arom-crane alread print log, this is only for set prop */
  if(!fuse_trust_boot_enabled()) {
    /*LOG_PRINT("### Non-trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "0");
  } else {
    /*LOG_PRINT("### Trusted boot mode. ###\n"); */
    asr_property_set("bl1.secureboot", "1");
  }
#else
  /*LOG_PRINT("### Non-trusted boot mode. ###\n"); */
  asr_property_set("bl1.secureboot", "0");
#endif

#if NOT_MCU_CR5
  /*
   * "halt": When download completed, if usb pull out, board powerdown. defaultly it is "halt",
   * "reboot": some customer want to reboot instead of powerdown, so customer set it to "reboot" manually.
   * "noaction": if vbus is unconnected, usb_connect() is 0.
   *             need set "noaction" to workaround the bug:
   *             After download, flasher judges that it is usb pulled out, enter forcedown by mistake.
   */
  asr_property_set("bl2.usb_disconnect", "halt");

  /* enter production mode after go(boot continue) if bl2.go_production is true */
#if SUPPORT_BOOT_GO_TO_PRODUCTION
  asr_property_set(PROP_NAME_GO_PRODUCTION, "true");
#endif

  usb_phy_config();

  fuse_read_lotid();

  if(hw_platform_type() == HW_PLATFORM_TYPE_SILICON) {

#ifdef QUECTEL_PREBOOT_OPT
    quec_powerkey_check();
#endif
	pmic_config();

  }
#endif
#endif

#if (SUPPORT_DDR_MEMORY)
  unsigned chip_id = hw_chip_id();
  if(chip_id == CHIP_ID_FULMAR || chip_id == CHIP_ID_CRANEGT) {
    ddr_init();
  }
#endif

#if (SUPPORT_PSRAM_MEMORY)
  psram_init();
#endif

#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
  /*
   * Watch1 cr5 psram is initialized in Harmony, and initialization is no longer repeated in
   * bootloader, mcu will write cr5 psram type to SW_SCRATCH(0xD4282CE8) register bit[15:8]
   * in Harmony, so cr5 read SW_SCRATCH bit[15:8] to get psram type here.
   */
  char psram_type_str[PROP_VALUE_MAX] = "";
  extern void psram_type_name_get_by_sw_scratch(char *psram_type_str, size_t size);
  psram_type_name_get_by_sw_scratch(psram_type_str, sizeof(psram_type_str));
  asr_property_set("fuse.psram.type", psram_type_str);
#endif

#if defined(CPU_MODEL_CRANE)
#if (AROM_VERSION != AROM_VER_2024_04_30_LAPWING_CR5)
  psram_mpu_config();
#endif
  /*
   * Atcm 64K is not enough to parse bootloder volume, so update to psram.
   */
  LOG_INFO("Set fip image load address spaces to 0x%x\n",
           (unsigned)&__lzma_decode_start);
  sb_fip_image_setup((void *)&__lzma_decode_start, (unsigned)&__lzma_decode_size);
#endif

#if (AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5) || (AROM_VERSION == AROM_VER_2023_09_09_FULMAR_CR5)
  uint32_t val = readl(CIU_BASE + SW_SCRATCH);
  val = (val & ~FLAGS_RUNNING_STAGE_MASK) | FLAGS_RUNNING_STAGE_PREBOOT_COMPLETE;
  writel(val, CIU_BASE + SW_SCRATCH);
  LOG_INFO("Set running stage: [0x%x] = 0x%x.\n", (CIU_BASE + SW_SCRATCH), (unsigned)readl(CIU_BASE + SW_SCRATCH));
#endif

  return 0;
}
/*---------------------------------------------------------------------------*/
