#include "efuse.h"
#include "reg.h"
#include <stdio.h>
#include <string.h>
#include "property.h"
#include "cpu.h"

#include "log.h"
#define LOG_MODULE "EFuse"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define PMU_MC_HW_SLP_TYPE                          (PMU_BASE + 0x0b0)
#define PMU_MC_CLK_RES_CTRL                         (PMU_BASE + 0x0f4)
#define PMU_CKPHY_FC_CTRL                           (PMU_BASE + 0x15c)

#define GEU_BASE                                    0xD4201000
#define GEU_BLOCK3_RESERVED_191_160                 (GEU_BASE + 0x04A4)

/* PMU AP */
#define PMU_AES_CLK_RES_CTRL                        (PMU_BASE + 0x68) /* AES Clock/Reset Control Register */

#define PMU_MAIN                                    0xD4050000
#define ACGR                                        (PMU_MAIN + 0x1024) /* Marvell Seagull/Mohawk Clock Gating Register */
#define APMU_104M_BIT                               12 /* bit12 */

#define SYS_BOOT_CNTRL                              (CIU_BASE + 0x20)
/*---------------------------------------------------------------------------*/
#define BLOCK_SIZE (32) /*bytes, 256bits */
#define GEU_BLOCK3_RESERVED_31_0                    (GEU_BASE + 0x0490)
/*#define GEU_BLOCK3_RESERVED_63_32                   (GEU_BASE + 0x0494) */
/*#define GEU_BLOCK3_RESERVED_95_64                   (GEU_BASE + 0x0498) */
/*#define GEU_BLOCK3_RESERVED_127_96                  (GEU_BASE + 0x049C) */
/*#define GEU_BLOCK3_RESERVED_159_128                 (GEU_BASE + 0x04A0) */
#define GEU_BLOCK3_RESERVED_191_160                 (GEU_BASE + 0x04A4)
#define GEU_BLOCK3_RESERVED_223_192                 (GEU_BASE + 0x041C)
#define GEU_BLOCK3_RESERVED_255_224                 (GEU_BASE + 0x0420)
/*---------------------------------------------------------------------------*/
/* PMU_AES_CLK_RES_CTRL */
typedef union {
  struct {
    unsigned aes_axi_reset                  : 1;        /* bit 0 */
    unsigned reserved0                      : 2;        /* bit 2:1 */
    unsigned aes_axi_clk_en                 : 1;        /* bit 3 */
    unsigned reserved1                      : 28;       /* bit 31:4 */
  } s;
  unsigned value;
} pmu_aes_clk_res_ctrl_t;
/*---------------------------------------------------------------------------*/
/* craneM A0 and craneG A0 */
typedef struct bank3_cranem_s {
  unsigned mem_repair_bit[5];                   /* bit 0~159*/
  /*160~191*/
  unsigned mem_repair_bit_6            : 8;     /* bit 160~167*/
  unsigned psram_edition               : 5;     /* bit 168~172*/
  unsigned device_version              : 2;     /* bit 173~174*/
  unsigned y_location                  : 7;     /* bit 175~181*/
  unsigned x_location                  : 7;     /* bit 182~188*/
#define FUSE_WAFERID_0_BITS         (3)
  unsigned waferid_0                   : 3;     /* bit 189~191*/
  /*192~223*/
  unsigned waferid_1                   : 2;     /* bit 192~193*/
  unsigned lotid5                      : 6;     /* bit 194~199*/
  unsigned lotid4                      : 6;     /* bit 200~205*/
  unsigned lotid3                      : 6;     /* bit 206~211*/
  unsigned lotid2                      : 6;     /* bit 212~217*/
  unsigned lotid1                      : 6;     /* bit 218~223*/
  /*224~255*/
  unsigned lotid0                      : 6;     /* bit 224~229*/
  unsigned hvtdro                      : 8;     /* bit 230~237*/
  unsigned lvtdro                      : 8;     /* bit 238~245*/
  unsigned svtdro                      : 8;     /* bit 246~253*/
  unsigned cp_good_die                 : 1;     /* bit 254 */
  unsigned fd_good_die                 : 1;     /* bit 255 */
} bank3_cranem_t;
/*---------------------------------------------------------------------------*/
typedef struct bank3_crane_s {
  unsigned mem_repair_bit[5];                   /* bit 0~159*/
  /*160~191*/
  unsigned mem_repair_bit_6            : 8;     /* bit 160~167*/
  unsigned psram_edition               : 3;     /* bit 168~170*/
  unsigned gooddie_baddie              : 1;     /* bit 171 */
  unsigned y_location                  : 7;     /* bit 172~178*/
  unsigned x_location                  : 7;     /* bit 179~185*/
  unsigned waferid                     : 5;     /* bit 186~190*/
#define FUSE_CHIPVER_0_BITS         (1)
  unsigned chipver_0                   : 1;     /* bit 191 */
  /*192~223*/
  unsigned chipver_1                   : 1;     /* bit 192 */
  unsigned lotid5                      : 7;     /* bit 193~199*/
  unsigned lotid4                      : 7;     /* bit 200~206*/
  unsigned lotid3                      : 7;     /* bit 207~213*/
  unsigned lotid2                      : 7;     /* bit 214~220*/
#define FUSE_LOTID1_0_BITS          (3)
  unsigned lotid1_0                    : 3;     /* bit 221~223*/
  /*224~255*/
  unsigned lotid1_1                    : 4;     /* bit 224~227*/
  unsigned lotid0                      : 7;     /* bit 228~234*/
  unsigned lvtdro                      : 8;     /* bit 235~242*/
  unsigned svtdro                      : 8;     /* bit 243~250*/
  unsigned fundryid                    : 3;     /* bit 251~253 */
  unsigned fuse_ver_id                 : 2;     /* bit 254~255 */
} bank3_crane_t;
/*---------------------------------------------------------------------------*/
/*CIU:  SYS_BOOT_CNTRL */
typedef union {
  struct {
    unsigned debug_en                       : 1;        /* bit 0 */
    unsigned ncsah                          : 4;        /* bit 4:1 */
    unsigned reserved_0                     : 1;        /* bit 5 */
    unsigned jtag_disable                   : 1;        /* bit 6 */
    unsigned reserved_1                     : 5;        /* bit 11:7 */
    unsigned uart_port                      : 1;        /* bit 12 */
    unsigned usb_port                       : 1;        /* bit 13 */
    unsigned reserved_2                     : 1;        /* bit 14 */
    unsigned bootfrom                       : 1;        /* bit 15 */
    unsigned usb_wakeup                     : 1;        /* bit 16 */
    unsigned no_nand_device                 : 1;        /* bit 17 */
    unsigned secure_key_access_disable      : 1;        /* bit 18 */
    unsigned download_disable               : 1;        /* bit 19 */
    unsigned sbe                            : 1;        /* bit 20 */
    unsigned sde                            : 1;        /* bit 21 */
    unsigned boot_pltfm_state               : 4;        /* bit 25:22 */
    unsigned reserved_3                     : 6;        /* bit 31:26 */
  } s;
  unsigned value;
} icu_sys_boot_cntrl_t;
/*---------------------------------------------------------------------------*/
static void
geu_enable_clk(void)
{
  unsigned int val = readl(ACGR);

  val |= (1 << APMU_104M_BIT);
  writel(val, ACGR);

  pmu_aes_clk_res_ctrl_t pmu_aes_clk_res_ctrl;
  pmu_aes_clk_res_ctrl.value = readl(PMU_AES_CLK_RES_CTRL);
  pmu_aes_clk_res_ctrl.s.aes_axi_clk_en = 1;
  pmu_aes_clk_res_ctrl.s.aes_axi_reset = 1;
  writel(pmu_aes_clk_res_ctrl.value, PMU_AES_CLK_RES_CTRL);
}
/*---------------------------------------------------------------------------*/
unsigned
efuse_psram(void)
{
  /*read or write geu, must enable clk at first, else, geu bank dump always be 0. */
  geu_enable_clk();
  unsigned value;
  value = readl(GEU_BLOCK3_RESERVED_191_160);

  /*LOG_INFO("fuse read GEU_BLOCK3_RESERVED_191_160, value=0x%x\n", value); */
  /*bit 168~172, 5bits*/
  uint8_t psram_flag = (value >> 8) & 0x1F;
  return psram_flag;
}
/*---------------------------------------------------------------------------*/
#define GEU_FUSE_VAL_APCFG1                         (GEU_BASE + 0x0404)
#define GEU_FUSE_VAL_APCFG3                         (GEU_BASE + 0x040C)
#define BANK0_EMBED_FLASH_BIT                       (28)
/*---------------------------------------------------------------------------*/
int
fuse_read_embed_flash(void)
{
  unsigned val = (readl(GEU_FUSE_VAL_APCFG3) >> 16) + ((readl(GEU_FUSE_VAL_APCFG1) & 0xffff) << 16);

  val = (val >> BANK0_EMBED_FLASH_BIT) & 0x1;
  if(val) {
    LOG_PRINT("Has embedded flash\n");
    asr_property_set("fuse.embeded_flash", "1");
  } else {
    LOG_PRINT("No embedded flash\n");
    asr_property_set("fuse.embeded_flash", "0");
  }

  return val;
}
/*---------------------------------------------------------------------------*/
/*
 * registers for bank3
 * GEU_BLOCK3_RESERVED_31_0                            (GEU_BASE + 0x0490)
 * GEU_BLOCK3_RESERVED_63_32                           (GEU_BASE + 0x0494)
 * GEU_BLOCK3_RESERVED_95_64                           (GEU_BASE + 0x0498)
 * GEU_BLOCK3_RESERVED_127_96                          (GEU_BASE + 0x049C)
 * GEU_BLOCK3_RESERVED_159_128                         (GEU_BASE + 0x04A0)
 * GEU_BLOCK3_RESERVED_191_160                         (GEU_BASE + 0x04A4)
 * GEU_BLOCK3_RESERVED_223_192                         (GEU_BASE + 0x041C)
 * GEU_BLOCK3_RESERVED_255_224                         (GEU_BASE + 0x0420)

 * MEM Repair Bits:168;                                bit 167:0
 * Manufacturing Parameter0: 88;                       bit 255:168
 */
int
fuse_read_bank3(void *data)
{
  unsigned *ptr = (unsigned *)data;

  /*geu_enable_clk(); */
  for(unsigned i = 0; i < 6; i++) {
    ptr[i] = readl(GEU_BLOCK3_RESERVED_31_0 + 4 * i);
  }

  ptr[6] = readl(GEU_BLOCK3_RESERVED_223_192);
  ptr[7] = readl(GEU_BLOCK3_RESERVED_255_224);

  return 0;
}
/*---------------------------------------------------------------------------*/
static void
fuse_read_lotid_cranem(void)
{
  unsigned buf[8];
  memset(buf, 0, BLOCK_SIZE);
  fuse_read_bank3(buf);

  bank3_cranem_t *ptr = (bank3_cranem_t *)buf;
  unsigned wafer = (ptr->waferid_1 << FUSE_WAFERID_0_BITS) | ptr->waferid_0;

  LOG_INFO("lotid[0..5]: %d %d %d %d %d %d\n", ptr->lotid0, ptr->lotid1, ptr->lotid2, ptr->lotid3, ptr->lotid4, ptr->lotid5);
  LOG_INFO("X_location: %d, Y_location: %d, Wafer: %d\n", ptr->x_location, ptr->y_location, wafer);
}
/*---------------------------------------------------------------------------*/
static void
fuse_read_lotid_crane(void)
{
  unsigned buf[8];
  memset(buf, 0, BLOCK_SIZE);
  fuse_read_bank3(buf);

  bank3_crane_t *ptr = (bank3_crane_t *)buf;
  unsigned lotid1 = (ptr->lotid1_1 << FUSE_LOTID1_0_BITS) | ptr->lotid1_0;

  LOG_INFO("lotid[0..5]: %d %d %d %d %d %d\n", ptr->lotid0, lotid1, ptr->lotid2, ptr->lotid3, ptr->lotid4, ptr->lotid5);
  LOG_INFO("X_location: %d, Y_location: %d, Wafer: %d\n", ptr->x_location, ptr->y_location, ptr->waferid);
}
/*---------------------------------------------------------------------------*/
void
fuse_read_lotid(void)
{
  unsigned int chip_id = hw_chip_id();

  switch(chip_id) {
  case CHIP_ID_CRANE:
  case CHIP_ID_CRANEG_Z2:
    fuse_read_lotid_crane();
    break;
  case CHIP_ID_CRANEG:
  case CHIP_ID_CRANEM:
  case CHIP_ID_CRANEL:
  default:
    fuse_read_lotid_cranem();
    break;
  }
}
/*---------------------------------------------------------------------------*/
/* bootrom operation mode, block[91:90], apconfig[75:74], APGFG3[11:10],  map to CIU:  SYS_BOOT_CNTRL[20] */
int
fuse_trust_boot_enabled(void)
{
  icu_sys_boot_cntrl_t sys_boot_ctrl;

  sys_boot_ctrl.value = readl(SYS_BOOT_CNTRL);

  //LOG_INFO("fuse: sbe = %d, sys_boot_ctrl=0x%x\n", sys_boot_ctrl.s.sbe, (unsigned int)sys_boot_ctrl.value);
  return sys_boot_ctrl.s.sbe != 0;
}
/*---------------------------------------------------------------------------*/
