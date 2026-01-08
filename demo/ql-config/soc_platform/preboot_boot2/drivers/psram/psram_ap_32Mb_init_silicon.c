#include <stdio.h>
#include <stdbool.h>
#include "log.h"
#include "clock.h"

#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN
#define REG32(addr) (*(volatile unsigned int *)(addr))

// lijin: 2018_08_16-12:06 add the define for mmap bit 25 issue in Z2 silicon, ECO to make 25 always 0
#define MMAP_ECO_DONE
//#define PSRAM_DUAL_CS
#define PS_TABLE_DEXC_IDX 21    // 0 disable
//#define PS_TABLE_DEXC_IDX 22  // 1 disable

#define APM32

//FPGA PLATFORM NEED THIS!!!
//#define FIX_LATENCY
//#define PSRAM_FPGA_PHY

#define HALF_LP_MODE
//#define PS_SLREF
//#define DIS_PSC_CACHE
enum {
    PHY_DRIVE_highZ = 0,
    PHY_DRIVE_400ohm,
    PHY_DRIVE_300ohm,
    PHY_DRIVE_240ohm,
    PHY_DRIVE_200ohm, //default value
    PHY_DRIVE_100ohm,
    PHY_DRIVE_67ohm,
    PHY_DRIVE_40ohm,
};

enum {
    APM_DRIVE_1_16ohm = 0,  //1/16
    APM_DRIVE_1_2ohm,       //1/2
    APM_DRIVE_1_4ohm,       //1/4
    APM_DRIVE_1_8ohm,       //default value  1/8
};

static const char *apm_drive_list[] = { "1_16ohm", "1_2ohm", "1_4ohm", "1_8ohm" };
static const char *phy_drive_list[] = { "highZ", "400ohm", "300ohm", "240ohm", "200ohm", "100ohm", "67ohm", "40ohm" };

#define PHY_DRIVE   PHY_DRIVE_200ohm
#define APM_DRIVE   APM_DRIVE_1_8ohm

#define APM_TBL_OFST    0
#define APM_MRB_OFST    0
#define APM_TBL_MASK    0xfffffffc
#define APM_MRB_MASK    0xfffffffc

void init_psram_apm32(int PSRAM_BASE)
{
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8008) = 0x2e43;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8070) = 0x93929190;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8074) = 0x97969594;
//REG_C: SEQ_TABLE // APM32M LUT0: RD 125!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8090) = 0x93088400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8094) = 0x20048b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8098) = 0x9b04;
//REG_C: SEQ_TABLE // APM32M LUT1: WR 125!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a0) = 0x93088480;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a4) = 0x97048b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a8) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT2: RD 200!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b0) = 0x93088400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b4) = 0x20068b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b04;
//REG_C: SEQ_TABLE // APM32M LUT3: WR 200!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c0) = 0x93088480;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c4) = 0x20028b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c8) = 0x9704;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT 'h4: WR MR0 for high!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d8) = 0x72006002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80dc) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT 'h6: WR MR4 for high!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f0) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f4) = 0x72016002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f8) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT 'h5: Global Reset!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e0) = 0x600144ff;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e4) = 0x44ff210a;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e8) = 0x21046001;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80ec) = 0x2838;
//REG_C: SEQ_TABLE // APM32M LUT 'h7: WR MR6 halfsleep entry low!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8100) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8104) = 0xc406c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8108) = 0x21047202;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x810c) = 0x2880;
//REG_C: SEQ_TABLE // APM32M LUT 'h8: CEN low halfsleep exit!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8110) = 0x60012808;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8114) = 0x2c046010;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT 'h9: RD MR!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8120) = 0x8b188440;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8124) = 0x9f062004;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
//REG_C: SEQ_TABLE // APM32M LUT 'ha: WR MR DATA0 low!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8130) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8134) = 0x7200;
//REG_C: SEQ_TABLE // APM32M LUT 'hb: WR MR DATA1 low!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8140) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8144) = 0x7201;
//REG_C: SEQ_TABLE // APM32M LUT 'hc: WR MR DATA2 low!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8150) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8154) = 0x7202;
//REG_C: SEQ_TABLE // APM32M LUT 'hd: WR MR6 halfsleep entry high!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8160) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8164) = 0xc406c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8168) = 0x72026002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x816c) = 0x2880;

/*CMT_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type DFC_NCH_T, start tb_index = 0x0, tbl_psc_sd=0!
/*CMT_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x5803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2;
/*CMT_C: DFC_TABLE*/ //  clear seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3;
/*CMT_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type DFC_CH_T, start tb_index = 0x8, tbl_psc_sd=0!
/*CMT_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x8;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x9;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xa;
/*CMT_C: DFC_TABLE*/ //  set seq_user_cmd.upd_fp=0, upd_fp_wr=1!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1400;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xb;
/*CMT_C: DFC_TABLE*/ //  Trigger User Sequence!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xc;
/*CMT_C: DFC_TABLE*/ //  Trig Sequence start on index=0xb!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x6;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x9030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xd;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x910b;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xe;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xf;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x10;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence done!
/*CMT_C: DFC_TABLE*/    //  set seq_user_cmd.upd_fp=1, upd_fp_wr=0!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1200;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x11;
/*CMT_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=1!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x618;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x12;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x13;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x14;
/*CMT_C: DFC_TABLE*/ //  wait dll_lock!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x15;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x16;
/*CMT_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x17;
/*CMT_C: DFC_TABLE*/ //  Trigger User Sequence!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x18;
/*CMT_C: DFC_TABLE*/ //  Trig Sequence start on index=0xa!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x9030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x19;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x910a;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1a;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1b;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1c;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence done!
/*CMT_C: DFC_TABLE*/    //  clear seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1d;
/*CMT_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x20, tbl_psc_sd=0!
/*CMT_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x21;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x22;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence!
/*CMT_C: DFC_TABLE*/    //  Trig Sequence start on index=0x7!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0xd;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x209030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x23;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x9107;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x208030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x24;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x25;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x26;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence done!
/*CMT_C: DFC_TABLE*/    //  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x27;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
/*CMT_C: DFC_TABLE*/ //  wait dll_lock!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
/*CMT_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence!
/*CMT_C: DFC_TABLE*/    //  Trig Sequence start on index=0x8!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x208030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
/*CMT_C: DFC_TABLE*/    //  Trigger User Sequence done!
/*CMT_C: DFC_TABLE*/    //  clear seq_user_cmd.blk_lfq!
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x30;

/*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = (0xf00013 & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_1[0]*/ REG32(PSRAM_BASE + 0x8028) = 0x0;
/*    fp_seq_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8020) = 0x5100100;
/*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = (0xf0801b & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_1[1]*/ REG32(PSRAM_BASE + 0x9028) = 0x0;
/*    fp_seq_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9020) = 0x7100302;
}

void init_psram_wbd955(int PSRAM_BASE)
{
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8008) = 0x2003;
//REG_C: SEQ_TABLE // WBD LUT2: RD 200!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b0) = 0x93188480;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b4) = 0x20048f10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b043407;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80bc) = 0x0;
//REG_C: SEQ_TABLE // WBD LUT3: WR 200!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c0) = 0x93188400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c4) = 0x20048f10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043407;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;
//REG_C: SEQ_TABLE // WBD LUT 'h4: WR CR0 125M!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c460;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c401;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d8) = 0xc400c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80dc) = 0xc404c4bf;        // update for drive
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;
//REG_C: SEQ_TABLE // WBD LUT 'h6: WR CR0 200M!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f4) = 0xc4bfcf10;        // update for drive
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c414;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
//REG_C: SEQ_TABLE // WBD LUT 'h7: WR MR6 hybrid sleep entry!
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8100) = 0xd318c460;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8104) = 0xc400cf10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c460;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x810c) = 0x0;

/*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = 0x4bf;        // update for drive
/*     fp_mr_data_1[0]*/ REG32(PSRAM_BASE + 0x8028) = 0x0;
/*    fp_seq_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8020) = 0x4100100;
/*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = 0x14bf;        // update for drive
/*     fp_mr_data_1[1]*/ REG32(PSRAM_BASE + 0x9028) = 0x0;
/*    fp_seq_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9020) = 0x7100302;

}

static int psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;
    unsigned PSRAM_BASE = 0xc0100000;

    // read MR register
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    read_data = REG32(PSRAM_BASE + 0x8038);     // this is the mr read data value
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8034), REG32(PSRAM_BASE + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8038), read_data);

    return read_data;
}

int psram_write_mr0(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;
    unsigned PSRAM_BASE = 0xc0100000;
	 unsigned data0 = 0;
    (void)mr_addr;
    (void)cs;

	data0 = REG32(PSRAM_BASE + 0x8024);
    //read cur_fp_wr
    read_data = REG32(PSRAM_BASE + 0x8000);

    //modify cur_fp_wr(bit12_13)  - - select mr_byte0
    read_data &= 0xffffcfff;
    REG32(PSRAM_BASE + 0x8000) = read_data;

    /*REG_C, Config PSC*/ /*
       fp_mr_data_0[0]*/REG32(PSRAM_BASE + 0x8024) = num;

    // write MR register
    //REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x104;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x8024) = data0;

    return read_data;
}

int psram_write_mr4(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;
    unsigned PSRAM_BASE = 0xc0100000;
	unsigned data0 = 0;
	(void)mr_addr;
	(void)cs;
	 
	data0 = REG32(PSRAM_BASE + 0x8024);
    //read cur_fp_wr
    read_data = REG32(PSRAM_BASE + 0x8000);

    //modify cur_fp_wr(bit12_13)  - - select mr_byte0
    read_data &= 0xffffcfff;
    REG32(PSRAM_BASE + 0x8000) = read_data;

    /*REG_C, Config PSC*/ /*
       fp_mr_data_0[0]*/REG32(PSRAM_BASE + 0x8024) = num;

    // write MR register
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x106;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x8024) = data0;

    return read_data;
}


void psram_init_ap_4MB_cfg(void) // lpddr2_400_init
{
    unsigned read_data;
    int i;
    unsigned device_strength = APM_DRIVE;
    unsigned phy_strength = PHY_DRIVE;
    unsigned PSRAM_BASE;

    LOG_INFO("PSRAM INIT ap 32Mb - - 1229 !!! \n");
    LOG_INFO("device strength = APM_DRIVE_%s.\n", apm_drive_list[device_strength]);
    LOG_INFO("phy strength = PHY_DRIVE_%s.\n", phy_drive_list[phy_strength]);

    //enable psram hclk
    REG32(0xd42828f4) = 0x3;
    REG32(0xd4282800 + 0x15c) |= 0x10000;   //hw control for disable pu for dll and phy,and phy_clk rdy bit[11]= = 0 and bit[18] = = 0

    PSRAM_BASE = 0xc0100000;

    REG32(0xd42828b0) |= (1 << 23) | (0x5 << 18); //config for clk

    // reset device
    // tRP RESET# low pulse width 1us
    // tRST Reset to CMD valid    2us
    REG32(PSRAM_BASE + 0x18000 ) = 0x1;
    for (i = 0; i < 5; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }
    REG32(PSRAM_BASE + 0x18000 ) = 0x5;
    for (i = 0; i < 40; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }

    // reload initial value
#ifdef APM32
    init_psram_apm32(PSRAM_BASE);
#elif defined(WBD955) || defined(WINBOND)
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (1 << 8) | 0x7;
    #ifdef  WBD955
    init_psram_wbd955(PSRAM_BASE);
    #endif  // WBD955 end
#else  // ~WINBOND
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (0 << 8) | 0x7;
#endif  // ~WINBOND end


#if defined(APM32) || defined(WBD955)
/*         mmap_cfg_cr*/                                        // REG32(PSRAM_BASE + 0x4) = 0x7e07300d;      // 4M each, 8M total
/*         mmap_cfg_cr*/ REG32(PSRAM_BASE + 0x4) = 0x7e08400d;  // 8M each, 16M total, for not update anycase
#endif

  #ifdef PSRAM_FPGA_PHY

    #ifdef FIX_LATENCY
      #ifdef WINBOND
        #ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xc7ff;           //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xc7ff;           //neg, fix
        #else
    REG32(PSRAM_BASE + 0x18008) = 0x47ff;           //pos, fix
    REG32(PSRAM_BASE + 0x19008) = 0x47ff;           //pos, fix
        #endif
      #else
        #ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xf000;           //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xf000;           //neg, fix
        #else
    REG32(PSRAM_BASE + 0x18008) = 0x7000;           //pos, fix
    REG32(PSRAM_BASE + 0x19008) = 0x7000;           //pos, fix
        #endif
      #endif
    #else
        #ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xb000;           //neg, viable
    REG32(PSRAM_BASE + 0x19008) = 0xb000;           //neg, viable
        #else
    REG32(PSRAM_BASE + 0x18008) = 0x3000;           //pos, viable
    REG32(PSRAM_BASE + 0x19008) = 0x3000;           //pos, viable
        #endif
    #endif

    #ifdef  APM32
    REG32(PSRAM_BASE + 0x18008) =  0x3f01;
    REG32(PSRAM_BASE + 0x19008) =  0x3f01;
    REG32(PSRAM_BASE + 0x18008) |= 0x10000;
    REG32(PSRAM_BASE + 0x19008) |= 0x10000;
    #endif

  #endif    // DDR_FPGA_PHY end

#ifdef FIX_LATENCY
#ifdef APM32
/*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = (0xf00033 & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = (0xf0803b & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
#elif defined(WBD955)
    // low freq
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80dc) = 0xc40cc4bf;        // update for drive
    // high freq
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c41c;
/*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = (0xcbf & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = (0x1cbf & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
#elif defined(WINBOND)
    // low freq
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80dc) = 0xc40bc49f;
    // high freq
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;
    REG32(PSRAM_BASE + 0x8024) = (0x0b9f & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x2b9f & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
#else // !WINBOND
      /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24054433;
    REG32(PSRAM_BASE + 0x8024) = (0x402b & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x2033 & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST);
#endif  // !WINBOND end
#endif  // FIX_LATENCY end

#ifdef DIS_PSC_CACHE
    //disable cache
    read_data = REG32(PSRAM_BASE + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(PSRAM_BASE + 0x4000) = read_data;
#else // ~DIS_PSC_CACHE
    // decrease flush_idle time
    REG32(PSRAM_BASE + 0x4000) |= 0x1;//cache line 128 byte
    read_data = REG32(PSRAM_BASE + 0x4020);
    read_data &= ~((1 << 9) | (0xff << 24));
    read_data |= ((1 << 9) | (0xf0 << 24));
    REG32(PSRAM_BASE + 0x4020) = read_data;

    REG32(PSRAM_BASE + 0x4008) |= (1 << 31); //enable psc top module clock gating
#endif

    //wait for dll status
    read_data = REG32(PSRAM_BASE + 0x18010);
    while ((read_data & 0x100) != 0x100) {
        read_data = REG32(PSRAM_BASE + 0x18010);
    }

    #ifdef PSC_CKB_AS_CK
    // phy_reg4[5]  enable clkb
    // phy_reg3[4]  clkb as clk, disable clk
    // phy_reg4[3]  apm32 en, if 1 override above
    //FP0
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= (1 << (24 + 5)) | (1 << (16 + 4));
    REG32(PSRAM_BASE + 0x18004) = read_data;
    // FP1
    read_data = REG32(PSRAM_BASE + 0x19004);
    read_data |= (1 << (16 + 4));
    REG32(PSRAM_BASE + 0x19004) = read_data;
    #endif  // PSC_CKB_AS_CK

    #ifdef APM32
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= (1 << (24 + 3)) | (1 << (24 + 6));
    REG32(PSRAM_BASE + 0x18004) = read_data;
    read_data = REG32(PSRAM_BASE + 0x19004);
    read_data |= (1 << (24 + 3)) | (1 << (24 + 6));
    REG32(PSRAM_BASE + 0x19004) = read_data;

    //exit halfsleep
    REG32(PSRAM_BASE + 0x8030) = 0x108;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    // global reset
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    /* usleep more than 500us */
    clock_wait_ms(1);
    // global reset
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    /* usleep more than 100us */
    clock_wait_ms(1);

//    LOG_INFO("read mr after global reset begin@@@@11111!!! \n");
//    psram_read_mr(0x0, 0);
//    psram_read_mr(0x1, 0);
//    psram_read_mr(0x2, 0);
//    psram_read_mr(0x4, 0);
//    psram_read_mr(0x6, 0);
//    LOG_INFO("read mr after global reset end @@@@11111!!! \n");
    #ifdef FIX_LATENCY
    REG32(PSRAM_BASE + 0x9030) = 0x4;
    REG32(PSRAM_BASE + 0x8030) = 0x810a;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    #endif // FIX_LATENCY end
    #elif defined(WBD955) || defined(WINBOND)
    // enable phy for winbond
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x18004) = read_data;

    // program MR for
    // 1. hybird read
    // 2. fix latency or variable latency
    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
#else // !WINBOND
#ifdef FIX_LATENCY
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x2405442b;
    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24054433;
#endif  // FIX_LATENCY
#endif  // !WINBOND

#ifdef APM32
    //enable DFC
    /*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ((0x1 << 24) | 0x1);
#endif // APM32 end

#ifdef SDF_ON
    read_data = REG32(0xd42828b0); //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
#elif defined(PSRAM_HIGH)
    read_data = REG32(0xd42828b0); //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
#endif

    REG32(0XC0110000 + 0x4010) &= 0xFFFFFFF7;

    REG32(0XC0110000 + 0x8008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0x8008) |= 0x0;
    REG32(0XC0110000 + 0x9008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0x9008) |= 0x0;
    REG32(0XC0110000 + 0xa008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0xa008) |= 0x0;
    REG32(0XC0110000 + 0xb008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0xb008) |= 0x0;

    read_data = REG32(PSRAM_BASE + 0x18004 );
    REG32(PSRAM_BASE + 0x18004 ) = (read_data & 0xfffffff8) | (PHY_DRIVE);
    read_data = REG32(PSRAM_BASE + 0x19004 );
    REG32(PSRAM_BASE + 0x19004 ) = (read_data & 0xfffffff8) | (PHY_DRIVE);
    psram_read_mr(0x0, 0);
    psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x4, 0);
    psram_read_mr(0x6, 0);

    unsigned mr0_data = 0;
    mr0_data = psram_read_mr(0x0, 0);
    mr0_data &= 0xff;
    if (mr0_data == 0x1B) {
       	psram_write_mr0(0x0, 0x0, 0x00f00013);  //use byte0=0x13 to mr0
		psram_write_mr4(0x4, 0x0, 0x00f00013);  //use byte1=0x00 to mr4
        LOG_INFO("HIGH MR0 MR4   @@@@!!!\n");

        LOG_INFO("After MRW --MR--HIGH!!! \n");
        psram_read_mr(0x0, 0);
        psram_read_mr(0x1, 0);
        psram_read_mr(0x2, 0);
        psram_read_mr(0x4, 0);
        psram_read_mr(0x6, 0);
    }

    LOG_INFO("FC done: reg@[0x%08X]=[0x%08X].\n", (unsigned)0xd42828b0, REG32(0xd42828b0));
    LOG_INFO("PSRAM INIT ap 32Mb END!!! \n");
}
