#include "efuse.h"
#include "reg.h"
#include <stdio.h>
#include <string.h>
#include "property.h"
#include "cpu.h"
#include "pmu.h"

#include "log.h"
#define LOG_MODULE "EFuse"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
#define PMU_MC_HW_SLP_TYPE                          (PMU_BASE + 0x0b0)
#define PMU_MC_CLK_RES_CTRL                         (PMU_BASE + 0x0f4)
#define PMU_CKPHY_FC_CTRL                           (PMU_BASE + 0x15c)

#define SYS_BOOT_CNTRL                              (CIU_BASE + 0x20)
/*---------------------------------------------------------------------------*/
#define GEU_FUSE_BLOCK_SIZE (32) /*bytes, 256bits */
#define GEU_FUSE_BANK0_207_192                      (GEU_BASE + 0x0418)
#define GEU_BLOCK3_RESERVED_31_0                    (GEU_BASE + 0x0490)
/*#define GEU_BLOCK3_RESERVED_63_32                   (GEU_BASE + 0x0494) */
/*#define GEU_BLOCK3_RESERVED_95_64                   (GEU_BASE + 0x0498) */
/*#define GEU_BLOCK3_RESERVED_127_96                  (GEU_BASE + 0x049C) */
/*#define GEU_BLOCK3_RESERVED_159_128                 (GEU_BASE + 0x04A0) */
#define GEU_BLOCK3_RESERVED_191_160                 (GEU_BASE + 0x04A4)
#define GEU_BLOCK3_RESERVED_223_192                 (GEU_BASE + 0x041C)
#define GEU_BLOCK3_RESERVED_255_224                 (GEU_BASE + 0x0420)
#define GEU_FUSE_VAL_APCFG1                         (GEU_BASE + 0x0404)
#define GEU_FUSE_VAL_APCFG3                         (GEU_BASE + 0x040C)
#define BANK0_EMBED_FLASH_BIT                       (28)
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
/* craneL A0*/
typedef struct bank3_cranel_a0_s {
  unsigned mem_repair_bit[5];                   /* bit 0~159*/
  /*160~191*/
  unsigned mem_repair_bit_6            : 8;     /* bit 160~167*/
  unsigned psram_edition               : 4;     /* bit 168~171*//* craneL A0 use bit93 to replace bank3 bit172 for msb of psram type */
  unsigned device_version              : 3;     /* bit 172~174*/
  unsigned y_location                  : 7;     /* bit 175~181*/
  unsigned x_location                  : 7;     /* bit 182~188*/
#define CRANEL_A0_FUSE_WAFERID_0_BITS         (3)
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
} bank3_cranel_a0_t;
/*---------------------------------------------------------------------------*/
/* uuid refer to sdk utilities.c Read_Crane_CPU_UID() */
/* craneLG: no psram_edition, add lotid_6 */
typedef struct bank3_cranelg_s {
  unsigned mem_repair_bit[5];                   /* bit 0~159*/
  /*160~191*/
  unsigned mem_repair_bit_6            : 8;     /* bit 160~167*/
  unsigned rsvd_0                      : 1;     /* bit 168 */
  unsigned y_location                  : 7;     /* bit 169~175*/
  unsigned x_location                  : 7;     /* bit 176~182*/
  unsigned waferid                     : 5;     /* bit 183~187*/
#define CRANEL_A0_FUSE_LOTID6_0_BITS         (4)
  unsigned lotid6_0                    : 4;     /* bit 188~192*/
  /*192~223*/
  unsigned lotid6_1                    : 2;     /* bit 192~193*/
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
} bank3_cranelg_t;
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
/*
 * chips:no psram phy, can use auto detect psram type
 * psram driver try windond read, apm read to get psram type and size
 * new chips which support auto_detect, no fuse bits for psram_type, eg: craneLG
 * cr5: fulmar use ddr instead of psram; watch1 z1 and A0 use psram no phy, support auto detect, no need fuse
 */
static bool
_is_chip_support_auto_detect(int *is_fuse_has_psram_type)
{
  *is_fuse_has_psram_type = 0;

  if(is_cranels() || is_cranelr()) {
    *is_fuse_has_psram_type = 1;
    return true;
  }

  /*new chip, no need fuse type. cranew cr5 currently has psram type in fuse bit, also can not use it. */
  if(is_cranelg() || is_cranelrh() || is_cranelb() || is_cranew() || is_watch1()) {
    return true;
  }

  return false;
}
bool
is_chip_support_auto_detect(void)
{
  int is_fuse_has_psram_type = 0;

  return _is_chip_support_auto_detect(&is_fuse_has_psram_type);
}
/*
 * craneG, craneM, craneL, craneLS, craneLR, cranew cr5
 * craneGT use this 6bits as DDR type
 */
unsigned int
efuse_get_psram_version(void)
{
  /* read or write geu, must enable clk at first, else, geu bank dump always be 0. */
  geu_clk_enable();
  unsigned value;
  uint8_t psram_flag;

  /* craneg/cranem/cranegt */
  value = readl(GEU_BLOCK3_RESERVED_191_160);

  /* LOG_INFO("fuse read GEU_BLOCK3_RESERVED_191_160, value=0x%x\n", value); */
  /* bit 168~172, 5bits*/
  psram_flag = (value >> 8) & 0x1F;

#ifdef BOARD_CRANE
  /* cranes Bank0 Bit207 as psram_type_bit5, eg: craneL/LR/LS, cranew CR5, craneGT ddr_type */
  value = readl(GEU_FUSE_BANK0_207_192) & 0xffff;
  value = (value >> 15) & 0x1;
  psram_flag |= (value << 5);
#endif

  if(CHIP_ID_CRANE == hw_chip_id()) {
    /*crane: Bit 170~168 */
    psram_flag &= 0x7;
  } else if(CHIP_ID_CRANEL == hw_chip_id() ||
            CHIP_ID_CRANELS == hw_chip_id() ||
            CHIP_ID_CRANELR == hw_chip_id()) {
    /*
     * MSB: Bank0 Bit[93]
     * LSB: Bank3 Bit 171~168
     */
    unsigned int apcfg3 = readl(GEU_FUSE_VAL_APCFG3);
    unsigned int cranel_psram_type_bit4 = (apcfg3 & (1 << 13)) ? 1 : 0;
    psram_flag = (cranel_psram_type_bit4 << 4) | (psram_flag & 0x2F);
  } else {
    bool is_psram_type_auto_detect = false;
    int is_fuse_has_psram_type = 0;

    is_psram_type_auto_detect = _is_chip_support_auto_detect(&is_fuse_has_psram_type);
    if(is_psram_type_auto_detect && (0 == is_fuse_has_psram_type)) {
      /*eg: craneLG, new chip supports auto detect psram type, no fuse bits */
      psram_flag = 0;
    }
  }

  return psram_flag;
}
/*---------------------------------------------------------------------------*/
/*
 * eg: craneGT has ddr instead of psram
 * not use efuse_get_psram_version(), Bit 172~168;
 * use auto detect ddr type method to get ddr size, efuse ddr type can be empty.
 * preboot set prop, flasher read prop to get ddr size.
 */
unsigned int
get_ddr_size(void)
{
  unsigned int ddr_size = DRAM_SIZE_32M;
  const char *prop_str;

  prop_str = asr_property_get("fuse.psram.type");   /*ddr also use it. */
  if(NULL == prop_str) {
    LOG_PRINT("get prop fuse.psram.type failed!\n");
    return ddr_size;
  }

  if(!strcmp(prop_str, DRAM_AP_32M_NAME)) {
    ddr_size = DRAM_SIZE_32M;
  } else if(!strcmp(prop_str, DRAM_AP_64M_NAME)) {
    ddr_size = DRAM_SIZE_64M;
  } else if(!strcmp(prop_str, DRAM_UniIC_32M_NAME)){
    ddr_size = DRAM_SIZE_32M;
  } else {
    LOG_PRINT("Unknow dram device\n");
  }

  return ddr_size;
}
/*
 * not use efuse_get_psram_version();
 * craneLR, no phy chip use auto detect psram type method to get psram size, efuse psram type is empty.
 * preboot set prop, flasher read prop to get psram size.
 */
unsigned int
get_psram_size(void)
{
  unsigned int psram_size = PSRAM_SIZE_8M;
  const char *prop_str;

  if(hw_platform_type() == HW_PLATFORM_TYPE_FPGA) {
    return PSRAM_SIZE_16M;
  }

  prop_str = asr_property_get("fuse.psram.type");
  if(NULL == prop_str) {
    LOG_PRINT("get prop fuse.psram.type failed!\n");
    return psram_size;
  }

  if(!strcmp(prop_str, WB_4MB_NAME) ||
     !strcmp(prop_str, AP_4MB_NAME) ||
     !strcmp(prop_str, WB_XCCELA_4M_NAME) ||
     !strcmp(prop_str, UNIIC_4M_NAME)
     ) {
    psram_size = PSRAM_SIZE_4M;
  } else if(!strcmp(prop_str, AP_8MB_NAME) ||
            !strcmp(prop_str, WB_8MB_NAME) ||
            !strcmp(prop_str, AP_250MHZ_8M_NAME) ||
            !strcmp(prop_str, WB_250MHZ_8M_NAME) ||
            !strcmp(prop_str, WB_XCCELA_8M_NAME)
            ) {
    psram_size = PSRAM_SIZE_8M;
  } else if(!strcmp(prop_str, AP_16M_NAME) ||
            !strcmp(prop_str, WB_16M_NAME) ||
            !strcmp(prop_str, AP_8M8M_NAME) ||
            !strcmp(prop_str, WB_8M8M_NAME) ||
            !strcmp(prop_str, AP_UHS_8M8M_NAME) ||
            !strcmp(prop_str, WB_250MHZ_8M8M_NAME) ||
            !strcmp(prop_str, AP_250MHZ_8M8M_NAME) ||
            !strcmp(prop_str, AP_250MHZ_16M_NAME) ||
            !strcmp(prop_str, WB_250MHZ_16M_NAME) ||
            !strcmp(prop_str, WB_XCCELA_8M8M_NAME) ||
            !strcmp(prop_str, UNIIC_16M_NAME)
            ) {
    psram_size = PSRAM_SIZE_16M;
  } else if(!strcmp(prop_str, WB_32M_NAME) ||
            !strcmp(prop_str, AP256Mb_1_2V_X16_CS0_NAME) ||
            !strcmp(prop_str, AP256Mb_1_2V_X16_CS1_NAME) ||
            !strcmp(prop_str, AP256Mb_1_2V_X16_CS0_CS1_NAME) ||
            !strcmp(prop_str, WB256Mb_1_8V_X16_CS0_NAME) ||
            !strcmp(prop_str, WB256Mb_1_8V_X16_CS1_NAME) ||
            !strcmp(prop_str, WB256Mb_1_8V_X16_CS0_CS1_NAME)) {
    psram_size = PSRAM_SIZE_32M;
  } else {
    LOG_PRINT("Unknow psram device\n");
  }

  return psram_size;
}
/*---------------------------------------------------------------------------*/
/* cranew mcu */
unsigned
efuse_cranew_mcu_psram(void)
{
  /*read or write geu, must enable clk at first, else, geu bank dump always be 0. */
  geu_clk_enable();
  unsigned value;

  value = readl(GEU_BLOCK3_RESERVED_31_0);

  /*LOG_INFO("fuse read GEU_BLOCK3_RESERVED_191_160, value=0x%x\n", value); */
  /*bit 11~15, 5bits*/
  uint8_t psram_flag = (value >> 11) & 0x1F;

  return psram_flag;
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

  /*geu_clk_enable(); */
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

  memset(buf, 0, GEU_FUSE_BLOCK_SIZE);
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

  memset(buf, 0, GEU_FUSE_BLOCK_SIZE);
  fuse_read_bank3(buf);

  bank3_crane_t *ptr = (bank3_crane_t *)buf;
  unsigned lotid1 = (ptr->lotid1_1 << FUSE_LOTID1_0_BITS) | ptr->lotid1_0;

  LOG_INFO("lotid[0..5]: %d %d %d %d %d %d\n", ptr->lotid0, lotid1, ptr->lotid2, ptr->lotid3, ptr->lotid4, ptr->lotid5);
  LOG_INFO("X_location: %d, Y_location: %d, Wafer: %d\n", ptr->x_location, ptr->y_location, ptr->waferid);
}
/*---------------------------------------------------------------------------*/
static void
fuse_read_lotid_cranel_A0(void)
{
  unsigned buf[8];

  memset(buf, 0, GEU_FUSE_BLOCK_SIZE);
  fuse_read_bank3(buf);

  bank3_cranel_a0_t *ptr = (bank3_cranel_a0_t *)buf;
  unsigned wafer = (ptr->waferid_1 << CRANEL_A0_FUSE_WAFERID_0_BITS) | ptr->waferid_0;

  LOG_INFO("lotid[0..5]: %d %d %d %d %d %d\n", ptr->lotid0, ptr->lotid1, ptr->lotid2, ptr->lotid3, ptr->lotid4, ptr->lotid5);
  LOG_INFO("X_location: %d, Y_location: %d, Wafer: %d\n", ptr->x_location, ptr->y_location, wafer);
}
/*---------------------------------------------------------------------------*/
static void
fuse_read_lotid_cranelg(void)
{
  unsigned buf[8];

  memset(buf, 0, GEU_FUSE_BLOCK_SIZE);
  fuse_read_bank3(buf);

  bank3_cranelg_t *ptr = (bank3_cranelg_t *)buf;
  unsigned lotid6 = (ptr->lotid6_1 << CRANEL_A0_FUSE_LOTID6_0_BITS) | ptr->lotid6_0;

  LOG_INFO("lotid[0..6]: %d %d %d %d %d %d %d\n", ptr->lotid0, ptr->lotid1, ptr->lotid2, ptr->lotid3, ptr->lotid4, ptr->lotid5, lotid6);
  LOG_INFO("X_location: %d, Y_location: %d, Wafer: %d\n", ptr->x_location, ptr->y_location, ptr->waferid);
}
/*---------------------------------------------------------------------------*/
void
fuse_read_lotid(void)
{
  unsigned int chip_id = hw_chip_id();

  switch(chip_id) {
  case CHIP_ID_CRANE:
    fuse_read_lotid_crane();
    break;
  case CHIP_ID_CRANEL:
  case CHIP_ID_CRANELS:
  case CHIP_ID_CRANELR:
    fuse_read_lotid_cranel_A0();
    break;
  case CHIP_ID_CRANELG:
  case CHIP_ID_CRANELRH:
  case CHIP_ID_CRANELB:
    fuse_read_lotid_cranelg();
    break;
  case CHIP_ID_CRANEG:
  case CHIP_ID_CRANEM:
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

  /*LOG_INFO("fuse: sbe = %d, sys_boot_ctrl=0x%x\n", sys_boot_ctrl.s.sbe, (unsigned int)sys_boot_ctrl.value); */
  return sys_boot_ctrl.s.sbe != 0;
}
/*---------------------------------------------------------------------------*/
