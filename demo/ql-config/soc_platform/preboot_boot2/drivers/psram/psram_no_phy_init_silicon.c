/*This init is used for no phy AP32Mb and WB32Mb. Don't need to define psram version first.  - - 2023/03/23 */

/*
 * enable fast fresh
 * craneLR      (1602C_010)   WB_4MB      (1602S_011)   WB_4MB
 *              (1602S_012)   AP_4MB      (1602C_013)   AP_4MB
 * craneLs_A0   (3607_012)    WB_16MB     (3607_011)    AP_16MB
 *              (1608)
 *              (1609S_010)   WB_8M       (1609S_011)   WB_8M
 *              (1609S_012)   AP_8M       (1609S_013)   AP_8M
 *
 * use new driv----------------------------------------------------
 * craneLG
 * craneLRH
 * craneLB
 * cranew(no use now)
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"
#include "clock.h"
#include "efuse.h"
#include "cpu.h"
#include "property.h"
#include "assert.h"

#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN
#define REG32(addr) (*(volatile unsigned int *)(addr))

#define write32(addr, val)                          (*(volatile unsigned int *)(addr) = (val))
#define read32(addr)                                (*(volatile unsigned int *)(addr))

static const char *cache_size_list[] = { "64B", "128B", "256B", "512B" };

static unsigned ap32Mb_no_phy_init(void);
static unsigned wb32Mb_no_phy_init(void);
static unsigned ap64Mb_no_phy_init(void);
static unsigned wb64Mb_no_phy_init(void);
static unsigned ap128Mb_no_phy_init(void);
static unsigned wb128Mb_no_phy_init(void);

#define PSRAM_CACHELINE_128B     1
#define PSRAM_CACHELINE_256B     2
#define PSRAM_BASE 0xC0100000

#define CACHELINE128_BST128    0
#define CACHELINE128_BST2048   1
#define CACHELINE256_BST2048   2
#define BURST_LENGTH_128       3
#define BURST_LENGTH_2048      4

#define APM_TBL_OFST    0
#define APM_MRB_OFST    0
#define APM_TBL_MASK    0xfffffffc
#define APM_MRB_MASK    0xfffffffc

#define WBD_TBL_OFST0   4
#define WBD_TBL_OFST1   20
#define WBD_MRB_OFST    4
#define WBD_TBL_MASK0   0xffffff8f
#define WBD_TBL_MASK1   0xff8fffff
#define WBD_MRB_MASK    0xffffff8f
static int fix_latency = 0;
static int psram_fpga_phy = 0;
static int dis_fast_miss_acc = 0;
static int dis_psc_cache = 0;
static int half_lp_mode = 0;
static int ps_slref = 0;
static int ps_table_dexc_idx = 21;
static unsigned version;

static unsigned wb_16M_cache_bst_type = 0;
static unsigned burst_length = 0;
static unsigned cacheline = PSRAM_CACHELINE_128B;

enum {
    APM32Mb_DRIVE_1_16ohm = 0,  //1/16
    APM32Mb_DRIVE_1_2ohm,       //1/2
    APM32Mb_DRIVE_1_4ohm,       //1/4
    APM32Mb_DRIVEE_1_8ohm,      //default value  1/8
};

enum {
    WBD32Mb_DRIVE_50ohm = 0,
    WBD32Mb_DRIVE_35ohm,
    WBD32Mb_DRIVE_100ohm,
    WBD32Mb_DRIVEE_200ohm,
};

enum {
    APM64Mb_DRIVE_25ohm = 0,
    APM64Mb_DRIVE_50ohm,
    APM64Mb_DRIVE_100ohm,
    APM64Mb_DRIVE_200ohm,
};

enum {
    WBD64Mb_DRIVE_34ohm_default = 0,
    WBD64Mb_DRIVE_115ohm,
    WBD64Mb_DRIVE_67ohm,
    WBD64Mb_DRIVE_46ohm,
    WBD64Mb_DRIVE_34ohm,
    WBD64Mb_DRIVE_27ohm,
    WBD64Mb_DRIVE_22ohm,
    WBD64Mb_DRIVE_19ohm,
};

enum {
    APM128Mb_DRIVE_25ohm = 0,
    APM128Mb_DRIVE_50ohm,
    APM128Mb_DRIVE_100ohm,
    APM128Mb_DRIVE_200ohm,
};

enum {
    WBD128Mb_DRIVE_25ohm = 0,
    WBD128Mb_DRIVE_50ohm,
    WBD128Mb_DRIVE_100ohm,
    WBD128Mb_DRIVE_200ohm,
    WBD128Mb_DRIVE_300ohm,
};

enum {
    UNIIC128Mb_DRIVE_25ohm = 0,
    UNIIC128Mb_DRIVE_50ohm,
    UNIIC128Mb_DRIVE_100ohm,
    UNIIC128Mb_DRIVE_200ohm,
};
// new board use drive param depend on SIPI result (signal integrity & power integrity)
enum {
    UNIIC32Mb_DRIVE_25ohm = 0,
    UNIIC32Mb_DRIVE_50ohm,
    UNIIC32Mb_DRIVE_100ohm,
    UNIIC32Mb_DRIVE_200ohm,
};
static unsigned APM32Mb_DRIVE = APM32Mb_DRIVEE_1_8ohm;
static unsigned WBD32Mb_DRIVE = WBD32Mb_DRIVE_50ohm;

static unsigned APM64Mb_DRIVE = APM64Mb_DRIVE_50ohm;
static unsigned WBD64Mb_DRIVE = WBD64Mb_DRIVE_46ohm;

static unsigned APM128Mb_DRIVE = APM128Mb_DRIVE_50ohm;
static unsigned WBD128Mb_DRIVE = WBD128Mb_DRIVE_50ohm;

static unsigned UNIIC128Mb_DRIVE = UNIIC128Mb_DRIVE_50ohm;
static unsigned UNIIC32Mb_DRIVE = UNIIC32Mb_DRIVE_50ohm;
/*AP32Mb */
#define APM32

/*WB32Mb */
#define WINBOND
#define WBD_32Mb
#define INIT_FP1
/* RF depend on it */
static void set_drive_ohm(void)
{
    if (is_cranels() || is_cranelr()) {
        //already release, can not change parameter.
        APM32Mb_DRIVE = APM32Mb_DRIVEE_1_8ohm;
        WBD32Mb_DRIVE = WBD32Mb_DRIVE_100ohm;
        APM64Mb_DRIVE = APM64Mb_DRIVE_200ohm;
        WBD64Mb_DRIVE = WBD64Mb_DRIVE_115ohm;
        APM128Mb_DRIVE = APM128Mb_DRIVE_50ohm;
        WBD128Mb_DRIVE = WBD128Mb_DRIVE_100ohm;
        UNIIC128Mb_DRIVE = UNIIC128Mb_DRIVE_100ohm;
        UNIIC32Mb_DRIVE = UNIIC32Mb_DRIVE_100ohm;
    }

    //printf("set_drive_ohm: APM32Mb_DRIVE=%d, WBD32Mb_DRIVE=%d, APM64Mb_DRIVE=%d, WBD64Mb_DRIVE=%d, APM128Mb_DRIVE=%d, WBD128Mb_DRIVE=%d\n", APM32Mb_DRIVE, WBD32Mb_DRIVE, APM64Mb_DRIVE, WBD64Mb_DRIVE, APM128Mb_DRIVE, WBD128Mb_DRIVE);
    //default use new parameter
    if (is_cranelrh() || is_cranelb() || is_cranelg()) {
        APM32Mb_DRIVE = APM32Mb_DRIVEE_1_8ohm;
        WBD32Mb_DRIVE = WBD32Mb_DRIVE_50ohm;
        APM64Mb_DRIVE = APM64Mb_DRIVE_50ohm;
        WBD64Mb_DRIVE = WBD64Mb_DRIVE_46ohm;
        APM128Mb_DRIVE = APM128Mb_DRIVE_50ohm;
        WBD128Mb_DRIVE = WBD128Mb_DRIVE_50ohm;
        UNIIC128Mb_DRIVE = UNIIC128Mb_DRIVE_50ohm;
        UNIIC32Mb_DRIVE = UNIIC32Mb_DRIVE_50ohm;
    }

    if (is_cranew() || is_watch1()) {
        APM32Mb_DRIVE = APM32Mb_DRIVEE_1_8ohm;
        WBD32Mb_DRIVE = WBD32Mb_DRIVE_50ohm;
        APM64Mb_DRIVE = APM64Mb_DRIVE_50ohm;
        WBD64Mb_DRIVE = WBD64Mb_DRIVE_46ohm;
        APM128Mb_DRIVE = APM128Mb_DRIVE_50ohm;
        WBD128Mb_DRIVE = WBD128Mb_DRIVE_50ohm;
        UNIIC128Mb_DRIVE = UNIIC128Mb_DRIVE_50ohm;
        UNIIC32Mb_DRIVE = UNIIC32Mb_DRIVE_50ohm;
    }
}

typedef unsigned (*psram_init_try_t)(void);
#define MAX_PSRAM_TYPE_NAME   32

typedef struct {
    char psram_type[MAX_PSRAM_TYPE_NAME];
    unsigned mr1_value;
    psram_init_try_t psram_init_try;
} psram_info_t;

static psram_info_t psram_info[NO_PHY_PSRAM_TYPE_MAX];
static psram_info_t *cur_psram_info;

static int init_psram_info(void)
{
    psram_info_t *p_psram_info;

    p_psram_info = &psram_info[NO_PHY_AP_4M];
    strncpy(p_psram_info->psram_type, AP_4MB_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0xC98D;
    p_psram_info->psram_init_try = ap32Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_WB_4M];
    strncpy(p_psram_info->psram_type, WB_4MB_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0x5F00;
    p_psram_info->psram_init_try = wb32Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_AP_8M];
    strncpy(p_psram_info->psram_type, AP_8MB_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0x938D;
    p_psram_info->psram_init_try = ap64Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_WB_8M];
    strncpy(p_psram_info->psram_type, WB_8MB_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0x860C;
    p_psram_info->psram_init_try = wb64Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_AP_16M];
    strncpy(p_psram_info->psram_type, AP_16M_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0xDD8D;
    p_psram_info->psram_init_try = ap128Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_WB_16M];
    strncpy(p_psram_info->psram_type, WB_16M_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0x960C;
    p_psram_info->psram_init_try = wb128Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_UNIIC_16M];
    strncpy(p_psram_info->psram_type, UNIIC_16M_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0xC59A;
    p_psram_info->psram_init_try = ap128Mb_no_phy_init;

    p_psram_info = &psram_info[NO_PHY_UNIIC_4M];
    strncpy(p_psram_info->psram_type, UNIIC_4M_NAME, MAX_PSRAM_TYPE_NAME);
    p_psram_info->mr1_value = 0xC19A;
    p_psram_info->psram_init_try = ap64Mb_no_phy_init;

    return 0;
}

//************************************!!!!!Config Modify here!!!!!************************************//
static void set_config_for_chip(void)
{
    fix_latency = 0;
    psram_fpga_phy = 0;
    dis_fast_miss_acc = 0;
    half_lp_mode = 1;
    ps_slref = 1;
    ps_table_dexc_idx = 21;// 0 disable

    wb_16M_cache_bst_type = CACHELINE128_BST128;

    dis_psc_cache = 0;
}

static void wb32Mb_write_cr0(void)
{
    unsigned read_data = 0;

    REG32(PSRAM_BASE + 0x8034) = 0x1000;
    REG32(PSRAM_BASE + 0x8030) = 0x10a;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
}


static int psram_write_mr0_ap32Mb(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;
	unsigned data0 = 0;
	
    (void)mr_addr;
    (void)cs;

	data0 = REG32(PSRAM_BASE + 0x8024);
    /*read cur_fp_wr */
    read_data = REG32(PSRAM_BASE + 0x8000);

    /*modify cur_fp_wr(bit12_13)  - - select mr_byte0 */
    read_data &= 0xffffcfff;
    REG32(PSRAM_BASE + 0x8000) = read_data;

    /*REG_C, Config PSC*/ /*
       fp_mr_data_0[0]*/REG32(PSRAM_BASE + 0x8024) = num;

    /* write MR register */
    /*REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23); */
    REG32(PSRAM_BASE + 0x8030) = 0x104;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
	REG32(PSRAM_BASE + 0x8024) = data0;
    return read_data;
}

static int psram_write_mr4_ap32Mb(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;
	unsigned data0 = 0;
	 
	data0 = REG32(PSRAM_BASE + 0x8024);
    /*read cur_fp_wr */
    read_data = REG32(PSRAM_BASE + 0x8000);

    /*modify cur_fp_wr(bit12_13)  - - select mr_byte0 */
    read_data &= 0xffffcfff;
    REG32(PSRAM_BASE + 0x8000) = read_data;

    /*REG_C, Config PSC*/ /*
       fp_mr_data_0[0]*/REG32(PSRAM_BASE + 0x8024) = num;

    /* write MR register */
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x106;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x8024) = data0;
    return read_data;
}

static int psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    /* read MR register */
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    read_data = REG32(PSRAM_BASE + 0x8038);     /* this is the mr read data value */
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8034), REG32(PSRAM_BASE + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8038), read_data);

    return read_data;
}

static void init_psram_apm32(void)
{
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8008) = 0x2e43;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8070) = 0x93929190;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8074) = 0x97969594;
/*REG_C: SEQ_TABLE // APM32M LUT0: RD 125! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8090) = 0x93088400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8094) = 0x20048b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8098) = 0x9b04;
/*REG_C: SEQ_TABLE // APM32M LUT1: WR 125! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a0) = 0x93088480;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a4) = 0x97048b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a8) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT2: RD 200! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b0) = 0x93088400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b4) = 0x20068b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b04;
/*REG_C: SEQ_TABLE // APM32M LUT3: WR 200! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c0) = 0x93088480;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c4) = 0x20028b10;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c8) = 0x9704;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT 'h4: WR MR0 for high! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80d8) = 0x72006002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80dc) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT 'h6: WR MR4 for high! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f0) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f4) = 0x72016002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80f8) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT 'h5: Global Reset! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e0) = 0x600144ff;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e4) = 0x44ff210a;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e8) = 0x21046001;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80ec) = 0x2838;
/*REG_C: SEQ_TABLE // APM32M LUT 'h7: WR MR6 halfsleep entry low! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8100) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8104) = 0xc406c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8108) = 0x21047202;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x810c) = 0x2880;
/*REG_C: SEQ_TABLE // APM32M LUT 'h8: CEN low halfsleep exit! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8110) = 0x60012808;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8114) = 0x2c046010;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT 'h9: RD MR! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8120) = 0x8b188440;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8124) = 0x9f062004;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
/*REG_C: SEQ_TABLE // APM32M LUT 'ha: WR MR DATA0 low! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8130) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8134) = 0x7200;
/*REG_C: SEQ_TABLE // APM32M LUT 'hb: WR MR DATA1 low! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8140) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8144) = 0x7201;
/*REG_C: SEQ_TABLE // APM32M LUT 'hc: WR MR DATA2 low! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8150) = 0xcb18c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8154) = 0x7202;
/*REG_C: SEQ_TABLE // APM32M LUT 'hd: WR MR6 halfsleep entry high! */
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8160) = 0xc400c4c0;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8164) = 0xc406c400;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8168) = 0x72026002;
/*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x816c) = 0x2880;

/*CMT_C: DFC_TABLE*/    /*  PROG_DFC_TABLE for Table Type DFC_NCH_T, start tb_index = 0x0, tbl_psc_sd=0! */
/*CMT_C: DFC_TABLE*/    /*  set seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x5803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2;
/*CMT_C: DFC_TABLE*/ /*  clear seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3;
/*CMT_C: DFC_TABLE*/    /*  PROG_DFC_TABLE for Table Type DFC_CH_T, start tb_index = 0x8, tbl_psc_sd=0! */
/*CMT_C: DFC_TABLE*/    /*  set seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x8;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x9;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xa;
/*CMT_C: DFC_TABLE*/ /*  set seq_user_cmd.upd_fp=0, upd_fp_wr=1! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1400;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xb;
/*CMT_C: DFC_TABLE*/ /*  Trigger User Sequence! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xc;
/*CMT_C: DFC_TABLE*/ /*  Trig Sequence start on index=0xb! */
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
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence done! */
/*CMT_C: DFC_TABLE*/    /*  set seq_user_cmd.upd_fp=1, upd_fp_wr=0! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1200;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x11;
/*CMT_C: DFC_TABLE*/ /*  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=1! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x618;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x12;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x13;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x14;
/*CMT_C: DFC_TABLE*/ /*  wait dll_lock! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x15;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x16;
/*CMT_C: DFC_TABLE*/ /*  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x17;
/*CMT_C: DFC_TABLE*/ /*  Trigger User Sequence! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x18;
/*CMT_C: DFC_TABLE*/ /*  Trig Sequence start on index=0xa! */
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
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence done! */
/*CMT_C: DFC_TABLE*/    /*  clear seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1d;
/*CMT_C: DFC_TABLE*/    /*  PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x20, tbl_psc_sd=0! */
/*CMT_C: DFC_TABLE*/    /*  set seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x21;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x22;
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence! */
/*CMT_C: DFC_TABLE*/    /*  Trig Sequence start on index=0x7! */
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
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence done! */
/*CMT_C: DFC_TABLE*/    /*  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x27;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
/*CMT_C: DFC_TABLE*/ /*  wait dll_lock! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
/*CMT_C: DFC_TABLE*/ /*  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence! */
/*CMT_C: DFC_TABLE*/    /*  Trig Sequence start on index=0x8! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x208030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x248030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
/*CMT_C: DFC_TABLE*/    /*  Trigger User Sequence done! */
/*CMT_C: DFC_TABLE*/    /*  clear seq_user_cmd.blk_lfq! */
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x30;

/*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = (0xf00013 & APM_MRB_MASK) | (APM32Mb_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_1[0]*/ REG32(PSRAM_BASE + 0x8028) = 0x0;
/*    fp_seq_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8020) = 0x5100100;
/*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = (0xf0801b & APM_MRB_MASK) | (APM32Mb_DRIVE << APM_MRB_OFST);
/*     fp_mr_data_1[1]*/ REG32(PSRAM_BASE + 0x9028) = 0x0;
/*    fp_seq_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9020) = 0x7100302;
}


static void winbond_32Mb_half_lp_mode(void)
{
    /*[prog_dfc_table_seq] PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x28 */
    /*[mcUvmRegDfcScdlrSeq] DFC_TB set seq_user_cmd.blk_lfq! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x20;

    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.0: Addr: 8030, Data: 1000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x21;

    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.1: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x22;

    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.2: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x23;

    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.3: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x24;

    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.4: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=1! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x83;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;

    /*[mcUvmRegUpdRWDSSeq] Write Reg Tb 5.5: Addr: 8008, Data: 83, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[uc_trig_seq_seq] DFC_TB Trigger User Sequence! */
    /*[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1002;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;

    /*[uc_trig_seq_seq] Write Reg Tb 5.6: Addr: 8034, Data: 1002, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x7! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x27;

    /*[uc_trig_seq_seq] Write Reg Tb 5.7: Addr: 8030, Data: 1107, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;

    /*[uc_trig_seq_seq] Write Reg Tb 6.0: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;

    /*[uc_trig_seq_seq] Write Reg Tb 6.1: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x7! */
    /*[uc_trig_seq_seq] DFC_TB Trigger User Sequence done! */
    /*[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.2: Addr: 8004, Data: 610, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.3: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.4: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 1, EOP: 0, RD=1 */
    /*[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=1, rx_bias_rbn=1! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.5: Addr: 8004, Data: 619, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.6: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.7: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[mcUvmRegWaitDllSeq] DFC_TB wait dll_lock! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;

    /*[mcUvmRegWaitDllSeq] Write Reg Tb 7.0: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;

    /*[mcUvmRegWaitDllSeq] Write Reg Tb 7.1: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;

    /*[mcUvmRegPhyOvrdSeq] Write Reg Tb 7.2: Addr: 8004, Data: 609, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[uc_trig_seq_seq] DFC_TB Trigger User Sequence! */
    /*[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0! */
    /*[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x8! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;

    /*[uc_trig_seq_seq] Write Reg Tb 7.3: Addr: 8030, Data: 1108, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;

    /*[uc_trig_seq_seq] Write Reg Tb 7.4: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;

    /*[uc_trig_seq_seq] Write Reg Tb 7.5: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1 */
    /*[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x8! */
    /*[uc_trig_seq_seq] DFC_TB Trigger User Sequence done! */
    /*[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=0! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;

    /*[mcUvmRegUpdRWDSSeq] Write Reg Tb 7.6: Addr: 8008, Data: 3, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0 */
    /*[mcUvmRegDfcScdlrSeq] DFC_TB clear seq_user_cmd.blk_lfq! */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x37;
    /*[mcUvmRegDfcScdlrSeq] Write Reg Tb 7.7: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 1, RD=0 */
}

static void init_psram_wbd32(void)
{
    unsigned read_data;
    int i;

#ifdef WINBOND
    for (i = 0; i < 1000; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }

    (void)read_data; //just for delay
    /* address remap */
    REG32(PSRAM_BASE + 0x8078) = 0x87868584;
    REG32(PSRAM_BASE + 0x807c) = 0x8b8a8988;
    REG32(PSRAM_BASE + 0x8080) = 0x8f8e8d8c;
    REG32(PSRAM_BASE + 0x8084) = 0x93929190;
    REG32(PSRAM_BASE + 0x8088) = 0x97969594;
    REG32(PSRAM_BASE + 0x808c) = 0x9b9a9998;
    REG32(PSRAM_BASE + 0x8060) = 0x4838281;

    /*[REG_C: SEQ_TABLE]*/  /* WBD LUT0: RD 125! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8090) = 0x93188480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8094) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8098) = 0x9b043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x809c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/  /* WBD LUT1: WR 125! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a0) = 0x93188400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a4) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a8) = 0x97043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80ac) = 0x0;
    /*[REG_C: SEQ_TABLE]*/  /* WBD LUT2: RD 200! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b0) = 0x93188480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x20058f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b043408;

#ifdef WBD_32Mb
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x20048f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b043407;
#endif // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80bc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/  /* WBD LUT3: WR 200! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x93188400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x20058f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043408;

#ifdef WBD_32Mb
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x20048f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043407;
#endif // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;

#ifdef PSRAM_DUAL_CS
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h4: WR CR0 125M! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c401;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0xc400c400;

    if (fix_latency == 1) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
    }

#ifdef WBD_32Mb
    if (fix_latency == 1) {
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40fc48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
        else {   // ~DIS_PSC_CACHE
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40cc48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
    }
    else {  // ~FIX_LATENCY
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc407c48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
        else {  // ~DIS_PSC_CACHE
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc404c48f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
    }
#endif  // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h6: WR CR0 200M! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc48fcf10 & WBD_TBL_MASK1) | (WBD32Mb_DRIVE << WBD_TBL_OFST1);

    if (fix_latency == 1) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c423;
    }

#ifdef WBD_32Mb
    if (fix_latency == 1) {
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c41f;
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c41c;
        }
    } // ~FIX_LATENCY
    else {
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c417;
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c414;
        }
    }
#endif  // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h7: WR MR6 hybird sleep entry! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc4ffcf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c4e2;

#ifdef  WBD_32Mb
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc400cf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c460;
#endif // WBD_32Mb

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* LUT 'h8: CEN low halfsleep exit! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x62012880;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h9: RD MR! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x931884e0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x9f043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x812c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'ha: WR MR DATA0! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0xf000cf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x2402f001;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x813c) = 0x0;

#else // ~PSRAM_DUAL_CS

    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h4: WR CR0 125M! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0x84008460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x84008401;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x84008400;
    if (fix_latency == 1) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840b848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8403848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
    }

#ifdef WBD_32Mb
    if (fix_latency == 1) {
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840f848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840c848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
    }
    else { // ~FIX_LATENCY
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8407848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8404848f & WBD_TBL_MASK0) | (WBD32Mb_DRIVE << WBD_TBL_OFST0);
        }
    }

#endif  // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h6: WR CR0 200M! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0x93188460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) =  (0x848f8f10 & WBD_TBL_MASK1) | (WBD32Mb_DRIVE << WBD_TBL_OFST1);

    if (fix_latency == 1) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402842b;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028423;
    }

#ifdef WBD_32Mb
    if (fix_latency == 1) {
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402841f;
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402841c;
        }
    }
    else { // ~FIX_LATENCY
        if (dis_psc_cache == 1) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028417;
        }
        else { // ~DIS_PSC_CACHE
               /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028414;
        }
    }
#endif  // WBD_32Mb end

    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h7: WR MR6 hybird sleep entry! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0x93188460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0x84ff8f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x240284e2;
    #ifdef  WBD_32Mb
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc400cf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c460;
    #endif  // WBD_32Mb
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
    //LOG_INFO("!!! WB32Mb psram hybrid sleep entry right @@@@  !!!  \n");

    /*[REG_C: SEQ_TABLE]*/      /* LUT 'h8: CEN low halfsleep exit! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x22012880;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'h9: RD MR! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x931884e0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x9f043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x812c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/      /* WBD LUT 'ha: WR MR DATA0! */
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0x93188460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0xb0008f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x2402b001;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x813c) = 0x0;

#endif // ~PSRAM_DUAL_CS end

    /* tcph and MR byte */
    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x06000302;

#ifdef WBD_32Mb
    if (fix_latency == 1) {
        if (dis_psc_cache == 1) {
            REG32(PSRAM_BASE + 0x8024) = (0x0f8f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
            REG32(PSRAM_BASE + 0x9024) = (0x1f8f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
        }
        else { // ~DIS_PSC_CACHE
            REG32(PSRAM_BASE + 0x8024) = (0x0c8f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
            REG32(PSRAM_BASE + 0x9024) = (0x1c8f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
        }
    }
    else {  // ~FIX_LATENCY
        if (dis_psc_cache == 1) {
            REG32(PSRAM_BASE + 0x8024) = (0x478f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
            REG32(PSRAM_BASE + 0x9024) = (0x578f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
        }
        else { // ~DIS_PSC_CACHE
            REG32(PSRAM_BASE + 0x8024) = (0x448f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
            REG32(PSRAM_BASE + 0x9024) = (0x548f & WBD_MRB_MASK) | (WBD32Mb_DRIVE << WBD_MRB_OFST);
        }
    }
#endif  // WBD_32Mb end
#endif

#ifdef WINBOND
    winbond_32Mb_half_lp_mode();
    /* / *[prog_dfc_table_seq]* / PROG_DFC_TABLE for Table Type DFC_NCH_T, start tb_index = 0x0 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x54084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x5;
    /* / *[prog_dfc_table_seq]* / PROG_DFC_TABLE for Table Type DFC_CH_T, start tb_index = 0x8 */
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x8;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x9;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xa;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xb;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xc;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1200;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xd;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x618;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xe;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xf;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x10;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x11;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x12;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x13;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x14;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x15;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x16;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x17;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x130a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x18;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x19;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x2000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x130b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1e;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1f;
#endif
}

static void apm_half_lp_mode_ps_slref(void)
{
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x21;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x22;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x23;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x24;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x110c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x27;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x37;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x38;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x110b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x39;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3c;
}

static unsigned ap32Mb_no_phy_init(void)
{
    unsigned vender_id = 0;
    unsigned read_data;

    init_psram_apm32();
    /* mmap_cfg_cr*/                                        /* REG32(PSRAM_BASE + 0x4) = 0x7e07300d;	    // 4M each, 8M total */
    /* mmap_cfg_cr*/ REG32(PSRAM_BASE + 0x4) = 0x7e08400d;  /* 8M each, 16M total, for not update anycase */


    /* phy config for APM32 */
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= (1 << (24 + 3)) | (1 << (24 + 6));
    REG32(PSRAM_BASE + 0x18004) = read_data;
    read_data = REG32(PSRAM_BASE + 0x19004);
    read_data |= (1 << (24 + 3)) | (1 << (24 + 6));
    REG32(PSRAM_BASE + 0x19004) = read_data;

    /* exit halfsleep */
    REG32(PSRAM_BASE + 0x8030) = 0x108;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    /* global reset */
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    clock_wait_ms(1);/* > 500us */


    /* global reset */
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    clock_wait_ms(1); /*100us */

    psram_read_mr(0x0, 0);
    vender_id = psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x4, 0);
    psram_read_mr(0x6, 0);

    unsigned mr0_data = 0;

    //LOG_INFO("ap32Mb read mr step2.\n");

    mr0_data = psram_read_mr(0x0, 0);
    mr0_data &= 0xff;

    //LOG_INFO("mr0_data = 0x%x.\n", mr0_data);

    if (mr0_data == 0x1B) {
        //LOG_INFO("Force write MR0 MR4 to the value of FP0!!!\n");
		psram_write_mr0_ap32Mb(0x0, 0x0, 0x00f00013);	 //use byte0=0x13 to mr0
		psram_write_mr4_ap32Mb(0x4, 0x0, 0x00f00013);	 //use byte1=0x00 to mr4

        psram_read_mr(0x0, 0);
        psram_read_mr(0x1, 0);
        psram_read_mr(0x2, 0);
        psram_read_mr(0x4, 0);
        psram_read_mr(0x6, 0);
    }

    //LOG_INFO("PSRAM INIT no phy ap 32Mb END!!!	\n");
    return vender_id;
}

static unsigned wb32Mb_no_phy_init(void)
{
    unsigned read_data;
    unsigned vender_id = 0;

    /*reload initial value */
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (1 << 8) | 0x7;
    init_psram_wbd32();

    /*MMAP */
#ifdef PSRAM_DUAL_CS
    REG32(PSRAM_BASE + 0x0004) = 0x7E09400D;
#else
    REG32(PSRAM_BASE + 0x0004) = 0x7E09000D;
#endif

    /*Set QOS  X1 =4, LTE = 0 */
    read_data = REG32(0xd4282c00 + 0x118);      /*DDRC_PORT_AXI_QOS */
    //LOG_INFO("Before REG32(0xd4282c00+0x118) = 0x%x\n", read_data);
    read_data &= ~((0xf << 28) | (0xf << 24));  /* X1 =4, LTE = 0 */
    read_data |= (0x4 << 28);
    REG32(0xd4282c00 + 0x118) = read_data;
    //LOG_INFO("After REG32(0xd4282c00+0x118) = 0x%x\n", read_data);

    /*Change lfq_config_cr	qos_2 & qos_3 */
    read_data = REG32(PSRAM_BASE + 0x400c);
    //LOG_INFO("Before REG32(PSRAM_BASE+0x400c)= 0x%x\n", read_data);
    read_data &= ~((0x3 << 4) | (0x3 << 6));
    read_data |= ((0x3 << 4) | (0x2 << 6));  /* set qos_2 = 3 & qos_3 = 2 */
    REG32(PSRAM_BASE + 0x400c) = read_data;
    //LOG_INFO("After REG32(PSRAM_BASE+0x400c)= 0x%x\n", read_data);

#ifdef WINBOND
    /* enable phy for winbond */
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x18004) = read_data;

    read_data = REG32(PSRAM_BASE + 0x19004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x19004) = read_data;

    /* program MR for */
    /* 1. hybird read */
    /* 2. fix latency or variable latency */
    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
#endif

    /* lijin: 2018_05_21-18:33 add for swdfc to fp1 */
#ifdef INIT_FP1
    REG32(PSRAM_BASE + 0x8000) |= 0x1 << 4;
    REG32(PSRAM_BASE + 0x8030) |= 0x1 << 9;

#ifdef WINBOND
    REG32(PSRAM_BASE + 0x8034) = 0x1000;
    REG32(PSRAM_BASE + 0x8030) = 0x106;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
#endif
#endif // INIT_FP1 end

    psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);
    //LOG_INFO("@@@@@@@@@!!!  \n");

    //write mr0 to add refresh to fast
    wb32Mb_write_cr0();
    vender_id = psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);
    //LOG_INFO("PSRAM INIT no phy wb 32Mb END!!!  \n");

    return vender_id;

}


static unsigned ap64Mb_no_phy_init(void)
{
    unsigned read_data;
    unsigned vender_id;

    // reload initial value
    REG32(PSRAM_BASE + 0x4024) &= ~((0xf << 16) | (0xf << 12) | (0xf << 8) | 0x7);
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (0 << 8) | 0x7;

    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x05000302;
    if (version == UNIIC_4M) {
        REG32(PSRAM_BASE + 0x8024) = (0x400b & APM_MRB_MASK) | (UNIIC32Mb_DRIVE << APM_MRB_OFST);
        REG32(PSRAM_BASE + 0x9024) = (0x2013 & APM_MRB_MASK) | (UNIIC32Mb_DRIVE << APM_MRB_OFST);
    }
    else {
        REG32(PSRAM_BASE + 0x8024) = (0x400b & APM_MRB_MASK) | (APM64Mb_DRIVE << APM_MRB_OFST);
        REG32(PSRAM_BASE + 0x9024) = (0x2013 & APM_MRB_MASK) | (APM64Mb_DRIVE << APM_MRB_OFST);
    }
    REG32(PSRAM_BASE + 0x8024) |= 0x480000;
    REG32(PSRAM_BASE + 0x9024) |= 0x280000;
    apm_half_lp_mode_ps_slref();

    // global reset for if initial for wdt reset
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x0004) = 0x7E08000D;

    psram_read_mr(0x0, 0);
    vender_id = psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x3, 0);
    psram_read_mr(0x4, 0);

    return vender_id;
}


//#define WB64Mb_PSRAM_REFRESH_SELECT
#ifdef WB64Mb_PSRAM_REFRESH_SELECT

static void psram_wr_testmode(void)
{
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'hb: WR TESTMODE!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8140) = 0xc400c440;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8144) = 0x4400cb20;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8148) = 0x0;
}

static int psram_write_testmode(unsigned seq_addr)
{
    unsigned read_data = 0;

    REG32(PSRAM_BASE + 0x8034) = seq_addr;
    REG32(PSRAM_BASE + 0x8030) = 0x10b;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    return read_data;
}

#if 0
static int winbond_psram_middle_refresh_entry(void)
{
    psram_write_testmode(0xA4A00008);   //STEP1
    psram_write_testmode(0x04000008);   //STEP2
    psram_write_testmode(0x04000008);   //STEP3
    psram_write_testmode(0xA4A00008);   //STEP4
    psram_write_testmode(0x14900008);   //STEP5
    psram_write_testmode(0x04800008);   //STEP6
    psram_write_testmode(0xA4A00008);   //STEP7
    psram_write_testmode(0x14B00008);   //STEP8
    psram_write_testmode(0x04C00008);   //STEP9

    return 0;
}

static int winbond_psram_middle_refresh_entry_test(void)
{
    unsigned reg = 0;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg |= (1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    REG32(PSRAM_BASE + 0x0004) = 0x7E68F001;
    winbond_psram_middle_refresh_entry();
    REG32(PSRAM_BASE + 0x0004) = 0x7E684001;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg &= ~(1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    //clock_wait_ms(100);
    //psram_read_mr(0x1002, 0);
    //psram_read_mr(0x1002, 1);

    return 0;
}
#endif
static int winbond_psram_fast_refresh_entry(void)
{
    psram_write_testmode(0xA4A00008);   //STEP1
    psram_write_testmode(0x04000008);   //STEP2
    psram_write_testmode(0x04000008);   //STEP3
    psram_write_testmode(0xA4A00008);   //STEP4
    psram_write_testmode(0x14900008);   //STEP5
    psram_write_testmode(0x04c00008);   //STEP6

    return 0;
}

static int winbond_psram_fast_refresh_entry_test(void)
{
    unsigned reg = 0;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg |= (1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    REG32(PSRAM_BASE + 0x0004) = 0x7E68F001;
    winbond_psram_fast_refresh_entry();
    REG32(PSRAM_BASE + 0x0004) = 0x7E684001;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg &= ~(1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    //clock_wait_ms(100);
    //psram_read_mr(0x1002, 0);
    //psram_read_mr(0x1002, 1);

    return 0;
}

#if 0
static int winbond_psram_fast_refresh_exit(void)
{
    psram_write_testmode(0xA4A00008);   //STEP1
    psram_write_testmode(0x54600008);   //STEP2
    psram_write_testmode(0x04000008);   //STEP3

    return 0;
}

static int winbond_psram_fast_refresh_exit_test(void)
{
    unsigned reg = 0;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg |= (1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    REG32(PSRAM_BASE + 0x0004) = 0x7E68F001;
    winbond_psram_fast_refresh_exit();
    REG32(PSRAM_BASE + 0x0004) = 0x7E684001;

    reg = REG32(PSRAM_BASE + 0x4010);
    reg &= ~(1 << 12);
    REG32(PSRAM_BASE + 0x4010) = reg;

    //clock_wait_ms(100);
    //psram_read_mr(0x1002, 0);
    //psram_read_mr(0x1002, 1);

    return 0;
}
#endif
#endif

static unsigned wb64Mb_no_phy_init(void)
{
    unsigned read_data, vender_id;

    // reload initial value
    REG32(PSRAM_BASE + 0x4024) &= ~((0xf << 16) | (0xf << 12) | (0xf << 8) | 0x7);
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (1 << 8) | 0x7;
    REG32(PSRAM_BASE + 0x8008) |= 0x10;

    // enable phy for winbond
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x18004) = read_data;

    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x06000302;
    REG32(PSRAM_BASE + 0x8024) = (0x39f & WBD_MRB_MASK) | (WBD64Mb_DRIVE << WBD_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x239f & WBD_MRB_MASK) | (WBD64Mb_DRIVE << WBD_MRB_OFST);


    // program MR for
    // 1. hybird read
    // 2. fix latency or variable latency
    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    read_data = REG32(PSRAM_BASE + 0x8000);
    read_data &= 0xff00ffff;
    read_data |= (4 << 16) | (4 << 20);
    REG32(PSRAM_BASE + 0x8000) = read_data;

    REG32(PSRAM_BASE + 0x0004) = 0x7E08000D;

    vender_id = psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);

    //Add WB64Mb psram refresh
    //psram_wr_testmode();
    //winbond_psram_fast_refresh_entry_test();

    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);

    return vender_id;
}

static unsigned ap128Mb_no_phy_init(void)
{
    unsigned vender_id, read_data;

    // reload initial value
    REG32(PSRAM_BASE + 0x4024) &= ~((0xf << 16) | (0xf << 12) | (0xf << 8) | 0x7);
    REG32(PSRAM_BASE + 0x4024) = (0 << 16) | (0 << 12) | (0 << 8) | 0x7;

    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x05000302;
    if (version == AP_16M) {
        REG32(PSRAM_BASE + 0x8024) = (0x400b & APM_MRB_MASK) | (APM128Mb_DRIVE << APM_MRB_OFST);
        REG32(PSRAM_BASE + 0x9024) = (0x2013 & APM_MRB_MASK) | (APM128Mb_DRIVE << APM_MRB_OFST);
    }
    else { //uniic 16M psram
        REG32(PSRAM_BASE + 0x8024) = (0x400b & APM_MRB_MASK) | (UNIIC128Mb_DRIVE << APM_MRB_OFST);
        REG32(PSRAM_BASE + 0x9024) = (0x2013 & APM_MRB_MASK) | (UNIIC128Mb_DRIVE << APM_MRB_OFST);
    }
    REG32(PSRAM_BASE + 0x8024) |= 0x480000;
    REG32(PSRAM_BASE + 0x9024) |= 0x280000;

    if (is_cranew() || is_watch1()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x110c;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;

        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x00004;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3b;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x110b;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3c;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3d;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3e;
    }
    // global reset for if initial for wdt reset
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x0004) = 0x7E08000D;

    psram_read_mr(0x0, 0);
    vender_id = psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x3, 0);
    psram_read_mr(0x4, 0);

    return vender_id;
}



static void winbond128Mb_config(void)
{
    unsigned i;

    static const char *type[] = { "CACHELINE128_BST128", "CACHELINE128_BST2048", "CACHELINE256_BST2048", "BURST_LENGTH_128", "BURST_LENGTH_2048" };

    LOG_INFO("[PSRAM] version:[%d]  dis_psc_cache:[%d].\n", version, dis_psc_cache);
    if (version == WB_16M) {
        LOG_INFO("[PSRAM] wb_16M_cache_bst_type:[%s]  burst_length:[%s]  .\n", type[wb_16M_cache_bst_type], type[burst_length]);
    }
    for (i = 0; i < 1000; i++) {}
    // address remap
    REG32(PSRAM_BASE + 0x8078) = 0x87868584;
    REG32(PSRAM_BASE + 0x807c) = 0x8b8a8988;
    REG32(PSRAM_BASE + 0x8080) = 0x8f8e8d8c;
    REG32(PSRAM_BASE + 0x8084) = 0x93929190;
    //REG32(PSRAM_BASE + 0x8088) = 0x97969594;
    if (version == WB_16M) {
        REG32(PSRAM_BASE + 0x8088) = 0x97969594; // mask bit 23 for winbond, so that it'll behave the same as APM when accidentally access 16MB ~32MB.
        REG32(PSRAM_BASE + 0x808c) = 0x9b9a9900;
    }
    else {
        REG32(PSRAM_BASE + 0x8088) = 0x00969594; // mask bit 23 for winbond, so that it'll behave the same as APM when accidentally access 16MB ~32MB.
        REG32(PSRAM_BASE + 0x808c) = 0x9b9a9998;
    }
    REG32(PSRAM_BASE + 0x8060) = 0x4838281;

    /*[REG_C: SEQ_TABLE]*/ // WBD LUT0: RD 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8090) = 0x93188480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8094) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8098) = 0x9b043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x809c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT1: WR 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a0) = 0x93188400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a4) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a8) = 0x97043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80ac) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT2: RD 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b0) = 0x93188480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x20058f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b043408;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80bc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT3: WR 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x93188400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x20058f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043408;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;

    if (version == WB_16M) {
        /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h4: WR CR0 125M!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c460;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c401;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0xc400c400;
        if (dis_psc_cache == 1) {
            if (fix_latency) {
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc49f & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //0xc40bc49f;//0xc40bc48f;
            }
            else {
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c49f & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //0xc403c49f;//0xc403c48f;
            }
        }
        else {
            if (fix_latency) {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc408c49f & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc49e & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //burst_length:2048
                }
            }
            else {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc400c49f & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c49e & WBD_TBL_MASK0) | (WBD128Mb_DRIVE << WBD_TBL_OFST0); //burst_length:2048
                }
            }
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;

        /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h6: WR CR0 200M!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
        if (burst_length == BURST_LENGTH_128) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc49fcf10 & WBD_TBL_MASK1) | (WBD128Mb_DRIVE << WBD_TBL_OFST1);    //burst_length:128 ok
        }
        else {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc49ecf10 & WBD_TBL_MASK1) | (WBD128Mb_DRIVE << WBD_TBL_OFST1);    //burst_length:2048
        }
        if (dis_psc_cache == 1) {
            if (fix_latency) {
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;
            }
            else {
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c423;
            }
        }
        else {
            if (fix_latency) {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c428;//burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;//burst_length:2048
                }
            }
            else {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c420;//burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c423;//burst_length:2048
                }
            }
        }

        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
    }

    /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h7: WR MR6 hybird sleep entry!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc4ffcf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c4e2;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/                                          // LUT 'h8: CEN low halfsleep exit!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x62012880;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210; //0x2d606210;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/                                          // WBD LUT 'h9: RD MR!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x931884e0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x20038f10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x9f043406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x812c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'ha: WR MR DATA0!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0xf000cf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x2402f001;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x813c) = 0x0;

    // tcph and MR byte
    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x06000302;

    if (version == WB_16M) {
        if (dis_psc_cache == 1) {
            if (fix_latency) {
                REG32(PSRAM_BASE + 0x8024) = (0x0b9f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);    //0xb9f; //0xb8f;
                REG32(PSRAM_BASE + 0x9024) = (0x2b9f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);    //0x2b9f;//0x2b8f;
            }
            else {
                REG32(PSRAM_BASE + 0x8024) = (0x039f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);    //0x39f;//0x38f;
                REG32(PSRAM_BASE + 0x9024) = (0x239f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);    //0x239f;//0x238f;
            }
        }
        else {
            if (fix_latency) {
                if (burst_length == BURST_LENGTH_128) {
                    REG32(PSRAM_BASE + 0x8024) = (0x089f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:128 ok
                    REG32(PSRAM_BASE + 0x9024) = (0x289f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:128 ok
                }
                else {
                    REG32(PSRAM_BASE + 0x8024) = (0x0b9e & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:2048
                    REG32(PSRAM_BASE + 0x9024) = (0x2b9e & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:2048
                }
            }
            else {
                if (burst_length == BURST_LENGTH_128) {
                    REG32(PSRAM_BASE + 0x8024) = (0x009f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:128 ok
                    REG32(PSRAM_BASE + 0x9024) = (0x209f & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:128 ok
                }
                else {
                    REG32(PSRAM_BASE + 0x8024) = (0x039e & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:2048
                    REG32(PSRAM_BASE + 0x9024) = (0x239e & WBD_MRB_MASK) | (WBD128Mb_DRIVE << WBD_MRB_OFST);        //burst_length:2048
                }
            }
        }

    }
}

static void winbond128Mb_half_lp_mode(void)
{
    //[prog_dfc_table_seq] PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x28
    //[mcUvmRegDfcScdlrSeq] DFC_TB set seq_user_cmd.blk_lfq!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.0: Addr: 8030, Data: 1000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.1: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.2: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.3: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.4: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x83;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 5.5: Addr: 8008, Data: 83, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence!
    //[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1002;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    //[uc_trig_seq_seq] Write Reg Tb 5.6: Addr: 8034, Data: 1002, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x7!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    //[uc_trig_seq_seq] Write Reg Tb 5.7: Addr: 8030, Data: 1107, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    //[uc_trig_seq_seq] Write Reg Tb 6.0: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
    //[uc_trig_seq_seq] Write Reg Tb 6.1: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x7!
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence done!
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.2: Addr: 8004, Data: 610, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.3: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.4: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 1, EOP: 0, RD=1
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=1, rx_bias_rbn=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.5: Addr: 8004, Data: 619, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.6: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x37;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.7: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegWaitDllSeq] DFC_TB wait dll_lock!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x38;
    //[mcUvmRegWaitDllSeq] Write Reg Tb 7.0: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x39;
    //[mcUvmRegWaitDllSeq] Write Reg Tb 7.1: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3a;
    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 7.2: Addr: 8004, Data: 609, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence!
    //[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0!
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x8!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3b;
    //[uc_trig_seq_seq] Write Reg Tb 7.3: Addr: 8030, Data: 1108, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3c;
    //[uc_trig_seq_seq] Write Reg Tb 7.4: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3d;
    //[uc_trig_seq_seq] Write Reg Tb 7.5: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x8!
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence done!
    //[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3e;
    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 7.6: Addr: 8008, Data: 3, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[mcUvmRegDfcScdlrSeq] DFC_TB clear seq_user_cmd.blk_lfq!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3f;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 7.7: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 1, RD=0
}


static void winbond128Mb_change_freq(void)
{
    // /*[prog_dfc_table_seq]*/ PROG_DFC_TABLE for Table Type DFC_NCH_T, start tb_index = 0x0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x54084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x5;
    // /*[prog_dfc_table_seq]*/ PROG_DFC_TABLE for Table Type DFC_CH_T, start tb_index = 0x8
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x8;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x9;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xa;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xb;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xc;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1200;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xd;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x618;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xe;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0xf;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x10;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x11;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x12;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x13;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x14;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x15;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x16;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x83;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x17;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x801000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x18;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x130a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x19;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x802000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x130b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1e;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x1f;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x21;
}


static unsigned wb128Mb_no_phy_init(void)
{
    unsigned vender_id, read_data;

    version = WB_16M;

    if ((version == WB_16M) && (dis_psc_cache == 0)) {
        if (wb_16M_cache_bst_type == CACHELINE256_BST2048) {//CACHELINE256_BST2048
            burst_length = BURST_LENGTH_2048;
            cacheline = PSRAM_CACHELINE_256B;
        }
        else if (wb_16M_cache_bst_type == CACHELINE128_BST2048) {//CACHELINE128_BST2048
            burst_length = BURST_LENGTH_2048;
            cacheline = PSRAM_CACHELINE_128B;
        }
        else { //CACHELINE128_BST128
            burst_length = BURST_LENGTH_128;
            cacheline = PSRAM_CACHELINE_128B;
        }
    }

    winbond128Mb_config();
    winbond128Mb_half_lp_mode();
    winbond128Mb_change_freq();

    // program MR for
    // 1. hybird read
    // 2. fix latency or variable latency
    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    read_data = REG32(PSRAM_BASE + 0x8000);
    read_data &= 0xff00ffff;
    read_data |= (5 << 16) | (5 << 20);
    REG32(PSRAM_BASE + 0x8000) = read_data;

    REG32(PSRAM_BASE + 0x0004) = 0x7E080001;

    vender_id = psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);

    return vender_id;
}

static void enable_psram_hclk(void)
{
    //enable psram hclk
    REG32(0xd42828f4) = 0x3;
    REG32(0xd4282800 + 0x15c) &= (~0x40800);     //hw control for disable pu for dll and phy,and phy_clk rdy bit[11]= = 0 and bit[18] = = 0
}

#if 1
/* added in psram.c psram_pll1_enable(), if this code updated, update psram_pll1_enable(), which both support phy and on phy */
void enable_pll_div(void)
{
    if ((is_cranels()) || (is_cranelr()) || (is_cranelrh()) || (is_cranelb()) || (is_cranelg())) {
        REG32(0xD4090000 + 0x12c) = 0x7;
        REG32(0xD4090000 + 0x104) = 0x3F;
    }

    if (is_cranew() || is_watch1()) {
        REG32(0xD4090000 + 0x104) = 0x3F;

        unsigned reg = 0;
        reg = read32(0x41000000 + 0x88);
        reg &= ~(0x3 << 13);
        reg |= (0x3 << 13);
        read32(0x41000000 + 0x88) = reg;
    }
}
#endif

static void psram_boot_from_fp0(void)
{
    unsigned read_data;

    if ((is_cranels()) || (is_cranelr()) || (is_cranelrh()) || (is_cranelb()) || (is_cranelg())) {
        //boot from low fp
        read_data = REG32(0xd42828b0);
        read_data &= ~((1 << 23) | (0x3 << 16) | (0x7 << 18) | (1 << 4));
        read_data |= (1 << 23) | (0x1 << 16) | (0x5 << 18);
        REG32(0xd42828b0) = read_data;
        read_data = REG32(0xd42828b0);
        while ((read_data & (1 << 23)) == (1 << 23)) {
            read_data = REG32(0xd42828b0);
        }
    }

    if (is_cranew() || is_watch1()) {
        //boot from low fp
        read_data = REG32(0xd42828b0);
        read_data &= ~((1 << 23) | (0x3 << 16) | (0x7 << 18) | (8 << 1) | (1 << 4));
        read_data |= (1 << 23) | (0x1 << 16) | (0x0 << 18) | (0x1 << 8);
        REG32(0xd42828b0) = read_data;
        read_data = REG32(0xd42828b0);
        while ((read_data & (1 << 23)) == (1 << 23)) {
            read_data = REG32(0xd42828b0);
        }
    }
}

static void psram_reset_device(void)
{
    unsigned read_data;
    int i;

    // reset device
    // tRP RESET# low pulse width 1us
    // tRST Reset to CMD valid	  2us
    REG32(PSRAM_BASE + 0x18000 ) = 0x1;
    for (i = 0; i < 5; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }
    REG32(PSRAM_BASE + 0x18000 ) = 0x5;
    for (i = 0; i < 40; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }

    (void)read_data; //just for delay
}

static void psram_enable_dfc(void)
{
    //enable DFC
    /*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ((0x1 << 24) | 0x1);
}

static void wait_for_dll_status(void)
{
    //wait for dll status
    unsigned read_data;

    if ((is_cranels()) || (is_cranelr()) || (is_cranelrh())) {
        read_data = REG32(PSRAM_BASE + 0x18010);
        while ((read_data & 0x100) != 0x100) {
            read_data = REG32(PSRAM_BASE + 0x18010);
        }
    }

    if ((is_cranelb()) || (is_cranelg())) {
        read_data = REG32(PSRAM_BASE + 0x18020);
        while ((read_data & 0x100) != 0x100) {
            read_data = REG32(PSRAM_BASE + 0x18020);
        }
    }

    if (is_cranew() || is_watch1()) {
        read_data = REG32(PSRAM_BASE + 0x18020);
        while ((read_data & 0x100) != 0x100) {
            read_data = REG32(PSRAM_BASE + 0x18020);
        }
    }
}

static void cache_config(void)
{
    if (dis_psc_cache) {
        //disable cache
        LOG_INFO("Disable psram cache\n");
        REG32(PSRAM_BASE + 0x4000) = REG32(PSRAM_BASE + 0x4000) & 0xff0f;
        LOG_INFO("Psram cache disabled !!!\n ");
    }
    else {
        unsigned reg_val = 0;

        REG32(PSRAM_BASE + 0x4000) |= cacheline;

        reg_val = REG32(PSRAM_BASE + 0x4020);
        reg_val &= ~((1 << 9) | (0xff << 24));
        reg_val |= ((1 << 9) | (0xf0 << 24));
        REG32(PSRAM_BASE + 0x4020) = reg_val;
        /*
         * 2020/12/18, lijin's suggestion for fix slient reset, which need be flushed(after row hammer
         * when FOTA is fixed). ASIC didn't add it, we need it.
         */
        REG32(PSRAM_BASE + 0x4008) |= (1 << 31);

        LOG_PRINT("Psram cache enabled !!! :cache size=[%s], @[0x%08X]=[0x%08X]\n",
                  cache_size_list[cacheline], (PSRAM_BASE + 0x4000), REG32(PSRAM_BASE + 0x4000));

        LOG_INFO("Psram cache : flush_idle @[0x%08X]=[0x%08X]\n", (PSRAM_BASE + 0x4020), REG32(PSRAM_BASE + 0x4020));
        LOG_INFO("Psram dis_psc_ckagate @[0x%08X]=[0x%08X]\n", (PSRAM_BASE + 0x4008), REG32(PSRAM_BASE + 0x4008));
    }
}

static void psc_phy_clk_rst_then_release(void)
{
    //reset psc_clk and phy_clk meanwhile and then release
    unsigned reg = 0;

    reg = read32(0xd42828b0);
    reg &= ~((1 << 30) | (1 << 21));
    read32(0xd42828b0) = reg;
    clock_wait_ms(5);/*  5ms */
    reg |= ((1 << 30) | (1 << 21));
    read32(0xd42828b0) = reg;
}

static unsigned set_io_drive_strength(void)
{
    unsigned reg = 0;

    //cranels_a0 /cranelr /cranew_cr5_psram
    if ((is_cranels()) || (is_cranelr()) || (is_cranelrh()) || (is_cranelb()) || (is_cranelg())) {
        //SIPI suggest value
        //reg = read32(0xd4090000+0x128);
        //reg &=(~ ((1<<0)|(1<<1)|(1<<6)));
        //read32(0xd4090000+0x128) = reg;

        reg = read32(0xd4090000 + 0x128);
        reg &= ~(0x3 << 28);
        reg |= (0x1 << 28);//change psram pad to pull down
        read32(0xd4090000 + 0x128) = reg;
        //LOG_INFO("Psram pad cfg @[0x%08X]=[0x%08X]\n", (0xd4090000 + 0x128), read32(0xd4090000 + 0x128));
    }
    else if (is_cranew() || is_watch1()) { /* cranew CR5 */
        //SIPI suggest value
        //reg = read32(0xd4090000+0x12c);
        //reg &=(~ ((1<<0)|(1<<1)|(1<<5)));
        //read32(0xd4090000+0x12c) = reg;

        reg = read32(0xd4090000 + 0x12c);
        reg &= ~(0x3 << 4);
        reg |= (0x1 << 4);//change psram pad to pull down
        read32(0xd4090000 + 0x12c) = reg;
        //LOG_INFO("Psram pad cfg @[0x%08X]=[0x%08X]\n", (0xd4090000 + 0x12c), read32(0xd4090000 + 0x12c));
    }

    return reg;
}

static void psram_die_info(unsigned psram_type)
{
    //LOG_INFO("Ap psram has good_die/bad_die info in mr2!!! But Winbond psram has no this info!!!\n");

    unsigned cs0_mr2_dara = 0;
    unsigned cs1_mr2_dara = 0;

    cs0_mr2_dara = psram_read_mr(0x2, 0);
    cs1_mr2_dara = psram_read_mr(0x2, 1);

    if ((psram_type == AP_4M) || (psram_type == AP_8M)) {
        if (((cs0_mr2_dara >> 7) & 0x1) == 0x1) {
            //LOG_INFO("PASS!!! AP CS0 good_die!!!\n");
        }
        else {
            LOG_INFO("FAIL!!! AP CS0 bad_die!!!\n");
        }
    }
    else if (psram_type == AP_8M8M) {
        if ((((cs0_mr2_dara >> 7) & 0x1) == 0x1) && (((cs1_mr2_dara >> 7) & 0x1) == 0x1)) {
            //LOG_INFO("PASS!!! AP CS0 and CS1 all good_die!!!\n");
        }
        else {
            if ((((cs0_mr2_dara >> 7) & 0x1) != 0x1) && (((cs1_mr2_dara >> 7) & 0x1) != 0x1)) {
                LOG_INFO("FAIL!!! AP CS0 and CS1 all bad_die!!!\n");
            }
            else if ((((cs0_mr2_dara >> 7) & 0x1) != 0x1) && (((cs1_mr2_dara >> 7) & 0x1) == 0x1)) {
                LOG_INFO("FAIL!!! AP CS0 bad_die only!!!\n");
            }
            else {
                LOG_INFO("FAIL!!! AP CS1 bad_die only!!!\n");
            }
        }
    }
    else if (psram_type == AP_16M) {
        if (((cs0_mr2_dara >> 5) & 0x7) == 0x6) { //2'b110
            //LOG_INFO("PASS!!! AP CS0 good_die\n");
        }
        else {
            LOG_INFO("FAIL!!! AP CS0 bad_die\n");
        }
    }
    else if (psram_type == UNIIC_16M) {
        if (((cs0_mr2_dara >> 5) & 0x7) == 0x6) { //2'b110
            //LOG_INFO("PASS!!! UniIC CS0 good_die\n");
        }
        else {
            LOG_INFO("FAIL!!! UniIC CS0 bad_die\n");
        }
    }
    else {
        LOG_INFO("This type psram has no die info!!!\n");
    }
}

static unsigned ap8M_or_ap8M8M_judge(void)
{
    unsigned cs0_mr1 = 0, cs0_mr2 = 0;
    unsigned cs1_mr1 = 0, cs1_mr2 = 0;

    cs0_mr1 = psram_read_mr(0x1, 0);
    cs0_mr2 = psram_read_mr(0x2, 0);

    cs1_mr1 = psram_read_mr(0x1, 1);
    cs1_mr2 = psram_read_mr(0x2, 1);

    if ((cs0_mr1 == 0x0000938D) && (cs0_mr2 == 0x0000A093) && (cs1_mr1 == 0x0000938D) && (cs1_mr2 == 0x0000A093)) {
        //LOG_INFO("This is AP_8M8M!!!\n");
        version = AP_8M8M;
    }

    //LOG_INFO("This is AP_8M!!!\n");
    version = AP_8M;
    return version;
}

static unsigned wb8M_or_wb8M8M_judge(void)
{
    unsigned cs0_mr1000 = 0, cs0_mr1002 = 0;
    unsigned cs1_mr1000 = 0, cs1_mr1002 = 0;

    cs0_mr1000 = psram_read_mr(0x1000, 0);
    cs0_mr1002 = psram_read_mr(0x1002, 0);

    cs1_mr1000 = psram_read_mr(0x1000, 1);
    cs1_mr1002 = psram_read_mr(0x1002, 1);

    if ((cs0_mr1000 == 0x0000039F) && (cs0_mr1002 == 0x0000C1FF) && (cs1_mr1000 == 0x0000039F) && (cs1_mr1002 == 0x0000C1FF)) {
        //LOG_INFO("This is WB_8M8M!!!\n");
        version = WB_8M8M;
    }
    else {
        //LOG_INFO("This is WB_8M!!!\n");
        version = WB_8M;
    }

    return version;
}

void psram_init_crane_no_phy_auto_detect(unsigned efuse_psram_type, char *psram_str)
{
    (void)efuse_psram_type;

    unsigned vender_id = 0;

    init_psram_info();

    version = efuse_psram_type;
#if 0
    printf("fake version=0");
    version = 0;
#endif
    set_drive_ohm();
    set_config_for_chip();

    enable_psram_hclk();

    psc_phy_clk_rst_then_release();

    if ((is_cranels()) || (is_cranelr()) || (is_cranew()) || (is_watch1()) || (is_cranelrh()) || (is_cranelb()) || (is_cranelg())) {
        enable_pll_div();
        psram_boot_from_fp0();
    }
    psram_reset_device();

    if (is_cranew() || is_watch1()) { /* cranew CR5 */
        //Set GPIO_16 to func2(cp_psram_rstn)
        unsigned reg = 0;
        reg = read32(0xD401E11C);
        reg &= ~((1 << 15) | (1 << 14) | 0X7);
        reg |= (1 << 15) | (1 << 14) | 0x2;
        write32(0xD401E11C, reg);
    }

    if (version == 0) {
        cur_psram_info = &psram_info[NO_PHY_AP_4M];
        vender_id = cur_psram_info->psram_init_try();

        if (vender_id != 0) {
            if ((vender_id & 0xffff) == cur_psram_info->mr1_value) {
                version = AP_4M;
                memcpy(psram_str,  cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_AP_8M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_AP_8M];
                version = AP_8M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();

                //LOG_INFO("AP64Mb Init PASS!!!  Single_cs or Dual_cs !!!\n");
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                version = ap8M_or_ap8M8M_judge();

                if (AP_8M == version) {
                    memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
                }
                else {
                    memcpy(psram_str, AP_8M8M_NAME, PROP_VALUE_MAX);
                }
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_AP_16M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_AP_16M];
                version = AP_16M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();
                memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_UNIIC_16M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_UNIIC_16M];
                version = UNIIC_16M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();  //Same as ap128Mb
                memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_UNIIC_4M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_UNIIC_4M];
                version = UNIIC_4M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();  //Same as ap64Mb
                REG32(PSRAM_BASE + 0x4) = 0x7e08400d;
                memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else {
                memcpy(psram_str, "UNKNOWN", PROP_VALUE_MAX);
                LOG_PRINT("No corresponding AP PSRAM found!!! vender_id = 0x%x\n", vender_id);
                assert(0);
            }
            psram_die_info(version);
        }
        else {
            psc_phy_clk_rst_then_release();

            cur_psram_info = &psram_info[NO_PHY_WB_4M];
            vender_id = cur_psram_info->psram_init_try();

            if ((vender_id & 0xffff) == cur_psram_info->mr1_value) {
                version = WB_4M;
                memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_WB_8M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_WB_8M];
                version = WB_8M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();

                //LOG_INFO("WB64Mb Init PASS!!!  Single_cs or Dual_cs !!!\n");
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                version = wb8M_or_wb8M8M_judge();

                if (WB_8M == version) {
                    memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
                }
                else {
                    memcpy(psram_str, WB_8M8M_NAME, PROP_VALUE_MAX);
                }
            }
            else if ((vender_id & 0xffff) == psram_info[NO_PHY_WB_16M].mr1_value) {
                cur_psram_info = &psram_info[NO_PHY_WB_16M];
                version = WB_16M;
                psc_phy_clk_rst_then_release();
                cur_psram_info->psram_init_try();

                memcpy(psram_str, cur_psram_info->psram_type, PROP_VALUE_MAX);
            }
            else {
                memcpy(psram_str, "UNKNOWN", PROP_VALUE_MAX);
                LOG_PRINT("No corresponding WB PSRAM found!!! vender_id = 0x%x\n", vender_id);
                assert(0);
            }
        }
    }
    else {
        /*burned fuse */
        switch (version) {
            case UNIIC_16M:
                version = UNIIC_16M;
                ap128Mb_no_phy_init();
                memcpy(psram_str, UNIIC_16M_NAME, PROP_VALUE_MAX);
                break;
            case AP_16M:
            case AP_250MHZ_16M:
                version = AP_16M;
                ap128Mb_no_phy_init();
                memcpy(psram_str, AP_16M_NAME, PROP_VALUE_MAX);
                break;
            case WB_16M:
            case WB_250MHZ_16M:
                version = WB_16M;
                wb128Mb_no_phy_init();
                memcpy(psram_str, WB_16M_NAME, PROP_VALUE_MAX);
                break;
            case AP_8M8M:
            case AP_250MHZ_8M8M:
                version = AP_8M8M;
                ap64Mb_no_phy_init();
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                memcpy(psram_str, AP_8M8M_NAME, PROP_VALUE_MAX);
                break;
            case WB_8M8M:
            case WB_250MHZ_8M8M:
                version = WB_8M8M;
                wb64Mb_no_phy_init();
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                memcpy(psram_str, WB_8M8M_NAME, PROP_VALUE_MAX);
                break;
            case AP_8M:
            case AP_250MHZ_8M:
                version = AP_8M;
                ap64Mb_no_phy_init();
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                memcpy(psram_str, AP_8MB_NAME, PROP_VALUE_MAX);
                break;
            case WB_8M:
            case WB_250MHZ_8M:
            case WB_XCCELA_8M:
                version = WB_8M;
                wb64Mb_no_phy_init();
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                memcpy(psram_str, WB_8MB_NAME, PROP_VALUE_MAX);
                break;
            case WB_4M:
                version = WB_4M;
                wb32Mb_no_phy_init();
                memcpy(psram_str, WB_4MB_NAME, PROP_VALUE_MAX);
                break;
            case AP_4M:
            case WB_XCCELA_4M:
                version = AP_4M;
                ap32Mb_no_phy_init();
                memcpy(psram_str, AP_4MB_NAME, PROP_VALUE_MAX);
                break;
            case UNIIC_4M:
                version = UNIIC_4M;
                ap64Mb_no_phy_init();
                REG32(PSRAM_BASE + 0x0004) = 0x7e08400d;
                memcpy(psram_str, UNIIC_4M_NAME, PROP_VALUE_MAX);
                break;

            default:
                memcpy(psram_str, "UNKNOWN", PROP_VALUE_MAX);
                LOG_PRINT("Haven't this type psram!!! Return!!! \n");
                assert(0);
                return;
        }
    }

    if (is_cranelb()) {
        REG32(PSRAM_BASE + 0x14008) &= (~((0x3 << 0) | (0x3 << 4)));
        REG32(PSRAM_BASE + 0x15008) &= (~((0x3 << 0) | (0x3 << 4)));
        REG32(PSRAM_BASE + 0x14008) |= (((0x1 << 0) | (0x3 << 4)));
        REG32(PSRAM_BASE + 0x15008) |= (((0x1 << 0) | (0x3 << 4)));

        LOG_INFO("set pipe@@@ addr[0x%x]:0x%x!!!  \n", PSRAM_BASE + 0x14008, REG32(PSRAM_BASE + 0x14008));
        LOG_INFO("set pipe@@@ addr[0x%x]:0x%x!!!  \n", PSRAM_BASE + 0x15008, REG32(PSRAM_BASE + 0x15008));
    }

    psram_enable_dfc();
    wait_for_dll_status();
    cache_config();

    REG32(0xc0110000 + 0x4010) &= ~(0x1 << 2);
    REG32(0xc0110000 + 0x4010) |= (0x1 << 2);

    //enbale pre and post for dq/dm
    REG32(0xc0110000 + 0x8004) |= (0x1 << 3) | (0x1 << 5);
    REG32(0xc0110000 + 0x9004) |= (0x1 << 3) | (0x1 << 5);

    //modify dll_range to little
    //LOG_INFO("before dll@[0x%X]=[0x%x]  \n",0xc0110000+ 0x8008,REG32(0xc0110000+ 0x8008));
    //REG32(0xc0110000+ 0x8008) &= ~(0x3<<8);
    //REG32(0xc0110000+ 0x8008) |= (0x0<<8);
    //LOG_INFO("after dll@[0x%X]=[0x%x]  \n",0xc0110000+ 0x8008,REG32(0xc0110000+ 0x8008));

    //set dqs_dly
    REG32(0xc0110000 + 0x8004) &= ~(0xf << 16);
    REG32(0xc0110000 + 0x8004) |= (0xe << 16);
    REG32(0xc0110000 + 0x9004) &= ~(0xf << 16);
    REG32(0xc0110000 + 0x9004) |= (0xe << 16);

    REG32(0xc0110000 + 0x8008) &= ~(0xff << 0);
    REG32(0xc0110000 + 0x8008) |= (0xb4 << 0);
    REG32(0xc0110000 + 0x9008) &= ~(0xff << 0);
    REG32(0xc0110000 + 0x9008) |= (0xb4 << 0);

#if 0
    void proper_dqs_dly_setting(void);
    proper_dqs_dly_setting();
#endif
    set_io_drive_strength();
}
