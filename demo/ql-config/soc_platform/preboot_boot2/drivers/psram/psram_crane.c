/******************************************************************************
*
*   THIS IS PSRAM INIT SILICON CODE.
*
*  PROJECT:
*           Crane            :  AP16M    AP8_8M    WB8_8M
*           CraneM           :  AP8M     AP8_8M    WB8M      WB8_8M
*           CraneG(non-uhs)  :  AP8M     AP8_8M    WB8M      WB8_8M
*           CraneL_A0        :  WB8M     WB_16M(3602s)
*           CraneW_CR5       :  AP16M
*
*
*
******************************************************************************/

#include "psram.h"
#include "efuse.h"
#include "cpu.h"
#include "property.h"
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "log.h"
#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define libEFUSE

#define write32(addr, val)                          (*(volatile unsigned int *)(addr) = (val))
#define read32(addr)                                (*(volatile unsigned int *)(addr))
#define GEU_FUSE_VAL_APCFG3                         (GEU_BASE + 0x040C)

/*
 * enable fast refresh rate. psram driver set lpm table, auto control refresh rate by lpm table:
 * AP8M/AP8M_8M/AP_16M; AP4M;
 * WB4MB: enable fast refresh rsee psram_wb_32Mb_init_silicon.c
 *
 * enable fast refresh rate. driver can not set lpm table for auto control. cp set fresh rate. cp: enter lpm, set low, exit lpm: set high.
 * WB8MB/WB8M_8M
 *
 * no need enable fast refresh rate. psram chip is low refresh as default, psram intenal design protects circuit to avoid bit flip.
 * WB_16M and WB_32M
 */
#define WB64Mb_PSRAM_REFRESH_SELECT

static bool is_crane(void);
static bool is_cranem(void);
static bool is_craneg(void);
static bool is_cranel(void); /* craneL z1 or A0 */
static bool is_cranel_a0(void);
bool is_cranels(void);
bool is_cranew(void);

/* CraneL/CraneW evb */
enum {
    CRANELW_PHY_DRIVE_highZ = 0,
    CRANELW_PHY_DRIVE_400ohm,
    CRANELW_PHY_DRIVE_300ohm,
    CRANELW_PHY_DRIVE_240ohm,
    CRANELW_PHY_DRIVE_200ohm, //default value
    CRANELW_PHY_DRIVE_100ohm,
    CRANELW_PHY_DRIVE_67ohm,
    CRANELW_PHY_DRIVE_40ohm,
};


// 20200406 temp add for test drive strength
// APM MR0[1:0]     0: 25ohm, 1: 50ohm(default), 2: 100 ohm, 3: 200 ohm
// WBD CR0[14:12]   0: 34ohm(default), 1: 115ohm, 2: 67ohm, 3: 46ohm, 4: 34ohm, 5: 27ohm, 6: 22ohm, 7: 19ohm
// phy reg1[2:0]    0: highZ, 1: 200ohm, 2: 100ohm, 3: 66ohm, 4: 50ohm, 5: 40ohm, 6: 33ohm, 7: 33ohm
enum {
    PHY_DRIVE_highZ = 0,
    PHY_DRIVE_200ohm,
    PHY_DRIVE_100ohm,
    PHY_DRIVE_66ohm,
    PHY_DRIVE_50ohm,
    PHY_DRIVE_40ohm,
    PHY_DRIVE_33ohm_default, // 6 and 7 are same 33ohm, 6 is default value
    PHY_DRIVE_33ohm,
};

enum {
    APM_DRIVE_25ohm = 0,
    APM_DRIVE_50ohm,
    APM_DRIVE_100ohm,
    APM_DRIVE_200ohm,
};

enum {
    WBD_DRIVE_34ohm_default = 0,
    WBD_DRIVE_115ohm,
    WBD_DRIVE_67ohm,
    WBD_DRIVE_46ohm,
    WBD_DRIVE_34ohm,
    WBD_DRIVE_27ohm,
    WBD_DRIVE_22ohm,
    WBD_DRIVE_19ohm,
};

enum {
    WBD16M_DRIVE_25ohm = 0,
    WBD16M_DRIVE_50ohm,
    WBD16M_DRIVE_100ohm,
    WBD16M_DRIVE_200ohm,
    WBD16M_DRIVE_300ohm,
};

#define APM_DRIVE      APM_DRIVE_200ohm
#define WBD_DRIVE      WBD_DRIVE_115ohm
#define WBD16M_DRIVE   WBD16M_DRIVE_100ohm

static int phy_drive = PHY_DRIVE_200ohm;

static const char *wbd_drive_list[] = { "34ohm_default", "115ohm", "67ohm", "46ohm", "34ohm", "27ohm", "22ohm", "19ohm" };
static const char *wbd16m_drive_list[] = { "25ohm", "50ohm", "100ohm", "200ohm", "300ohm" };

static const char *ap_drive_list[] = { "25ohm", "50ohm", "100ohm", "200ohm" };
static const char *phy_drive_list[] = { "highZ", "200ohm", "100ohm", "66ohm", "50ohm", "40ohm", "33ohm_default", "33ohm" };
static const char *cranelw_phy_drive_list[] = { "highZ", "400ohm", "300ohm", "240ohm", "200ohm", "100ohm", "67ohm", "40ohm" };

static char psram_str[PROP_VALUE_MAX] = "";

void set_phy_drive(void)
{
    if (is_cranel() ||
        is_cranels() ||
        is_cranew()) {
        phy_drive = CRANELW_PHY_DRIVE_200ohm;
        LOG_INFO("Crane L/W set phy_drive=CRANELW_PHY_DRIVE_%s \n", cranelw_phy_drive_list[phy_drive]);
    }
    else {
        phy_drive = PHY_DRIVE_200ohm;
        LOG_INFO("set phy_drive=PHY_DRIVE_%s \n", phy_drive_list[phy_drive]);
    }
}

// NO change below
#define APM_TBL_OFST    0
#define APM_MRB_OFST    0
#define WBD_TBL_OFST0   4
#define WBD_TBL_OFST1   20
#define WBD_MRB_OFST    4
#define APM_TBL_MASK    0xfffffffc
#define APM_MRB_MASK    0xfffffffc
#define WBD_TBL_MASK0   0xffffff8f
#define WBD_TBL_MASK1   0xff8fffff
#define WBD_MRB_MASK    0xffffff8f

#define REG32(addr) (*(volatile unsigned int *)(addr))
#define writel(v, a) (REG32(a) = (v))
#define readl(a) (REG32(a))

#define PSRAM_BASE  (0xc0100000)

/*define AP 16M device version */
#define ENGINEERING_MR2 7
#define MP_MR2 5
#define MP8M_MR2 3

static int chip_id = -1;
static int ver_id = -1;

static int fix_latency = 0;
static int psram_fpga_phy = 0;
static int dis_fast_miss_acc = 0;
static int dis_psc_cache = 0;
static int half_lp_mode = 0;
static int ps_slref = 0;
static int ps_table_dexc_idx = 0;
static const char *cache_size_list[] = { "64B", "128B", "256B", "512B" };
static unsigned version = 0;

//3602s WB16M use
#define CACHELINE128_BST128    0
#define CACHELINE128_BST2048   1
#define CACHELINE256_BST2048   2
#define BURST_LENGTH_128       3
#define BURST_LENGTH_2048      4

static unsigned wb_16M_cache_bst_type = 0;
static unsigned burst_length = 0;
static unsigned cacheline = PSRAM_CACHELINE_128B;

static void set_config_for_chip(void)
{
    if (is_crane()) {
        fix_latency = 0;
        psram_fpga_phy = 0;
        dis_fast_miss_acc = 0;
        dis_psc_cache = 0;
        half_lp_mode = 0;
        ps_slref = 0;
        ps_table_dexc_idx = 0;
    }
    else {
        fix_latency = 0;
        psram_fpga_phy = 0;
        dis_fast_miss_acc = 0;
        dis_psc_cache = 0;
        half_lp_mode = 1;
        ps_slref = 1;
        ps_table_dexc_idx = 21;// 0 disable
        //ps_table_dexc_idx = 22;// 1 disables
        wb_16M_cache_bst_type = CACHELINE128_BST128;
    }

    set_phy_drive();
}

static bool is_crane(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANE == chip_id) ? true : false;
}

static bool is_cranem(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANEM == chip_id) ? true : false;
}

static bool is_craneg(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANEG == chip_id) ? true : false;
}

/* craneL Z1 or A0 */
static bool is_cranel(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANEL == chip_id) ? true : false;
}

/* 3602s cranel a0 */
static bool is_cranel_a0(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    if (ver_id < 0) {
        ver_id = hw_rev_id();
    }

    return ((CHIP_ID_CRANEL == chip_id) && (0xA0 == ver_id)) ? true : false;
}

/* craneL s */
bool is_cranels(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANELS == chip_id) ? true : false;
}

bool is_cranelr(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANELR == chip_id) ? true : false;
}

bool is_cranelg(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANELG == chip_id) ? true : false;
}

bool is_cranelrh(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANELRH == chip_id) ? true : false;
}

bool is_cranelb(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANELB == chip_id) ? true : false;
}

bool is_cranew(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_CRANEW == chip_id || CHIP_ID_FULMAR == chip_id) ? true : false;
}

bool is_watch1(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return (CHIP_ID_WATCH1 == chip_id) ? true : false;
}

static void winbond_half_lp_mode(void)
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
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 5.5: Addr: 8008, Data: 83, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence!
    //[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1002;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    //[uc_trig_seq_seq] Write Reg Tb 5.6: Addr: 8034, Data: 1002, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x7!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    //[uc_trig_seq_seq] Write Reg Tb 5.7: Addr: 8030, Data: 1107, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    //[uc_trig_seq_seq] Write Reg Tb 6.0: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
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
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3b;
    //[uc_trig_seq_seq] Write Reg Tb 7.3: Addr: 8030, Data: 1108, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3c;
    //[uc_trig_seq_seq] Write Reg Tb 7.4: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3d;
    //[uc_trig_seq_seq] Write Reg Tb 7.5: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x8!
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence done!
    //[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3e;
    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 7.6: Addr: 8008, Data: 3, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[mcUvmRegDfcScdlrSeq] DFC_TB clear seq_user_cmd.blk_lfq!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x3f;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 7.7: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 1, RD=0
}

static void winbond_non_half_lp_mode(void)
{
    // /*[prog_dfc_table_seq]*/ PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x28
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;
}

static void winbond_half_sleep(void)
{
    if (half_lp_mode) {
        winbond_half_lp_mode();
    }
    else {
        winbond_non_half_lp_mode();
    }
}

static void winbond_change_freq(void)
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

static void apm_change_freq(void)
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
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
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
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
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
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
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
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    if (is_crane()) {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    }
    else {
        /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    }
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

static void apm_half_lp_mode_non_ps_slref(void)
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
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x27;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
}


static void apm_half_lp_mode(void)
{
    if (ps_slref) {
        apm_half_lp_mode_ps_slref();
    }
    else {
        if (is_cranem() ||
            is_cranel() ||
            is_cranels() ||
            is_craneg() ||
            is_cranew()) {
            apm_half_lp_mode_non_ps_slref();
        }
    }
}

static void apm_non_half_lp_mode(void)
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
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x27;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
}

static void apm_half_sleep(void)
{
    if (half_lp_mode) {
        apm_half_lp_mode();
    }
    else {
        apm_non_half_lp_mode();
    }
}

static void psram_type_name_get_by_id(unsigned psram_type, char *psram_type_str, size_t size)
{
    if (size > PROP_VALUE_MAX) {
        size = PROP_VALUE_MAX;
    }

    switch (psram_type) {
        case AP_16M:
            memcpy(psram_type_str, "AP_16M", size);
            break;
        case WB_16M:
            memcpy(psram_type_str, "WB_16M", size);
            break;
        case AP_8M8M:
            memcpy(psram_type_str, "AP_8M8M", size);
            break;
        case WB_8M8M:
            memcpy(psram_type_str, "WB_8M8M", size);
            break;
        case UNIIC_16M:
            memcpy(psram_type_str, "UNIIC_16M", size);
            break;
        case AP_8M:
            memcpy(psram_type_str, "AP_8M", size);
            break;
        case WB_8M:
            memcpy(psram_type_str, "WB_8M", size);
            break;
        case WB_250MHZ_8M:
            memcpy(psram_type_str, "WB_250MHZ_8M", size);
            break;
        case AP_UHS_8M8M:
            memcpy(psram_type_str, "AP_UHS_8M8M", size);
            break;
        case WB_250MHZ_8M8M:
            memcpy(psram_type_str, "WB_250MHZ_8M8M", size);
            break;
        case AP_250MHZ_8M8M:
            memcpy(psram_type_str, "AP_250MHZ_8M8M", size);
            break;
        case WB_XCCELA_8M:
            memcpy(psram_type_str, "WB_XCCELA_8M", size);
            break;
        case WB_XCCELA_8M8M:
            memcpy(psram_type_str, "WB_XCCELA_8M8M", size);
            break;
        case WB_4M:
            memcpy(psram_type_str, "WB_4M", size);
            break;
        case AP_4M:
            memcpy(psram_type_str, "AP_4M", size);
            break;
        case WB_XCCELA_4M:
            memcpy(psram_type_str, "WB_XCCELA_4M", size);
            break;
        case AP_250MHZ_8M:
            memcpy(psram_type_str, "AP_250MHZ_8M", size);
            break;
        case WB_32M:
            memcpy(psram_type_str, "WB_32M", size);
            break;
        case AP_250MHZ_16M:
            memcpy(psram_type_str, "AP_250MHZ_16M", size);
            break;
        case WB_250MHZ_16M:
            memcpy(psram_type_str, "WB_250MHZ_16M", size);
            break;
        default:
            LOG_PRINT("Not supported psram type.\n");
            assert(0);
            break;
    }
}

#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
void psram_type_name_get_by_sw_scratch(char *psram_type_str, size_t size)
{
    unsigned psram_type = FLAGS_PSRAM_TYPE_GET(readl(CIU_BASE + SW_SCRATCH));

    psram_type_name_get_by_id(psram_type, psram_type_str, size);
}
#endif

#ifdef libEFUSE
/* only craneL_A0:
 *  PSRAM_TYPE_BIT_4: bank0 Bit[93], AP_config Bit[77], which replace Bank3 Bit[172]
 *  GEU_FUSE_VAL_APCFG3 Bit15~0: ap[79:64], Bit16~31: cp[15:0]
 *  GEU_FUSE_VAL_APCFG3 Bit[13] is ap[77], PSRAM_TYPE_BIT_4.
 */
static unsigned psram_fuse(void)
{
    /*craneG or craneM: Bit 172~168 */
    unsigned psram_type = efuse_get_psram_version();

#if 0
    // fake for sv test without fuse burn.
    psram_type = AP_16M;
    LOG_PRINT("fake No fuse bit to test auto detect\n");
    //LOG_PRINT("No fuse bit , fake it to WB_8M instead of AP_16M\n");
#endif

    psram_type_name_get_by_id(psram_type, psram_str, sizeof(psram_str));

    return psram_type;
}
#endif

static int psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

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

    //delay_ms(100);
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

    //delay_ms(100);
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

    //delay_ms(100);
    //psram_read_mr(0x1002, 0);
    //psram_read_mr(0x1002, 1);

    return 0;
}
#endif
#endif

static int psram_rd_mr(void)
{
    unsigned read_data = 0;
    unsigned device = 0;

    REG32(PSRAM_BASE + 0x8030) = 0x104;//0x110
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    REG32(PSRAM_BASE + 0x8034) = 2 + (0 << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x109;//MR Read 0x115
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    read_data = REG32(PSRAM_BASE + 0x8038);
    device = read_data & 0x07;
    LOG_INFO("########################\n");
    if (device == MP_MR2) {
        LOG_INFO("MP DEVICE\n");
        LOG_INFO("MR[%d] VALUE = 0x%x\n", 2, device);
    }
    else if (device == ENGINEERING_MR2) {
        LOG_INFO("ENGINEERING DEVICE\n");
        LOG_INFO("MR[%d] VALUE = 0x%x\n", 2, device);
    }
    else if (device == MP8M_MR2) {
        LOG_INFO("8M DEVICE\n");
        LOG_INFO("MR[%d] VALUE = 0x%x\n", 2, device);
    }
    else {
        //LOG_INFO("NOT SUPPORTED DEVICE\n");
        LOG_INFO("MR[%d] VALUE = 0x%x\n", 2, device);
    }
    LOG_INFO("########################\n");
    return device;
}

static void clk_config(void)
{
    unsigned i;

    //enable psram hclk
    REG32(0xd42828f4) = 0x3;
    REG32(0xd4282800 + 0x15c) |= 0x10000;     //hw control for disable pu for dll and phy,and phy_clk rdy bit[11]= = 0 and bit[18] = = 0

    if (is_cranem() ||
        is_cranel() ||
        is_cranels() ||
        is_crane()) {
        REG32(0xd42828b0) |= (1 << 23) | (0x5 << 18);//config for clk
    }

    if (is_cranew()) {
        REG32(0xd42828b0) |= (1 << 23) | (0x0 << 18);//config for clk
    }

    REG32(PSRAM_BASE + 0x18000 ) = 0x1;
    for (i = 0; i < 200; i++) {}
    REG32(PSRAM_BASE + 0x18000 ) = 0x5;
}

static void phy_reg_config(void)
{
    unsigned read_data;

    if (is_cranem() ||
        is_cranel() ||
        is_cranels() ||
        is_craneg() ||
        is_cranew()) {
        // disable dll code auto update
        REG32(0XC0110000 + 0x4010) &= 0xFFFFFFF7;

        REG32(0XC0110000 + 0x8008) &= 0xFFFFFF00;
        REG32(0XC0110000 + 0x8008) |= 0x0;
        REG32(0XC0110000 + 0x9008) &= 0xFFFFFF00;
        REG32(0XC0110000 + 0x9008) |= 0x0;
        REG32(0XC0110000 + 0xa008) &= 0xFFFFFF00;
        REG32(0XC0110000 + 0xa008) |= 0x0;
        REG32(0XC0110000 + 0xb008) &= 0xFFFFFF00;
        REG32(0XC0110000 + 0xb008) |= 0x0;
    }

    if (is_cranel_a0() ||
        is_cranels() ||
        is_cranew()) {
        LOG_PRINT("CraneW & CraneL_A0 & CraneL_S: phy config\n");
        if ((version == WB_8M8M) || (version == WB_8M) || (version == WB_16M) || (version == WB_250MHZ_8M) || (version == WB_250MHZ_8M8M)) {
            REG32(PSRAM_BASE + 0x18004 ) = 0x93000498 | (phy_drive);
            REG32(PSRAM_BASE + 0x19004 ) = 0x93000498 | (phy_drive);
        }
        else if ((version == AP_8M8M) || (version == AP_8M) || (version == AP_16M) || (version == AP_250MHZ_8M) || (version == AP_250MHZ_8M8M)) {
            REG32(PSRAM_BASE + 0x18004 ) = 0x83000498 | (phy_drive);
            REG32(PSRAM_BASE + 0x19004 ) = 0x83000498 | (phy_drive);
        }
        else if (version == AP_4M) {
            /*REG32(PSRAM_BASE + 0x18004 ) = 0x8b00049c; */
            /*REG32(PSRAM_BASE + 0x19004 ) = 0x8b00049c; */
            /*this type have alone psram init code! not use this file!!! */
        }
    }
    else {
        read_data = REG32(PSRAM_BASE + 0x18004 );
        REG32(PSRAM_BASE + 0x18004 ) = (read_data & 0xfffffff8) | (phy_drive);  //0x1;
        read_data = REG32(PSRAM_BASE + 0x19004 );
        REG32(PSRAM_BASE + 0x19004 ) = (read_data & 0xfffffff8) | (phy_drive);  //0x1;
    }
}

static void winbond_config(void)
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
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc49f & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //0xc40bc49f;//0xc40bc48f;
            }
            else {
                /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c49f & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //0xc403c49f;//0xc403c48f;
            }
        }
        else {
            if (fix_latency) {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc408c49f & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc49e & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //burst_length:2048
                }
            }
            else {
                if (burst_length == BURST_LENGTH_128) {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc400c49f & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //burst_length:128 ok
                }
                else {
                    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c49e & WBD_TBL_MASK0) | (WBD16M_DRIVE << WBD_TBL_OFST0); //burst_length:2048
                }
            }
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;

        /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h6: WR CR0 200M!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
        if (burst_length == BURST_LENGTH_128) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc49fcf10 & WBD_TBL_MASK1) | (WBD16M_DRIVE << WBD_TBL_OFST1);    //burst_length:128 ok
        }
        else {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc49ecf10 & WBD_TBL_MASK1) | (WBD16M_DRIVE << WBD_TBL_OFST1);    //burst_length:2048
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
    else {
        /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h4: WR CR0 125M!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c460;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c401;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0xc400c400;
        if (fix_latency) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40bc49f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0); //0xc40bc49f;//0xc40bc48f;
        }
        else {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c49f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0); //0xc403c49f;//0xc403c48f;
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;

        /*[REG_C: SEQ_TABLE]*/                                                                                              // WBD LUT 'h6: WR CR0 200M!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc49fcf10 & WBD_TBL_MASK1) | (WBD_DRIVE << WBD_TBL_OFST1);    //0xc49fcf10;//0xc48fcf10;
        if (fix_latency) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;
        }
        else {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c423;
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
    }

    /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h7: WR MR6 hybird sleep entry!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0xd318c460;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc4ffcf10;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c4e2;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h8: CEN low halfsleep exit!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x62012880;
    if (is_crane()) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2d606210;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210; //0x2d606210;
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h9: RD MR!
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
                REG32(PSRAM_BASE + 0x8024) = (0x0b9f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);  //0xb9f; //0xb8f;
                REG32(PSRAM_BASE + 0x9024) = (0x2b9f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);  //0x2b9f;//0x2b8f;
            }
            else {
                REG32(PSRAM_BASE + 0x8024) = (0x039f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);  //0x39f;//0x38f;
                REG32(PSRAM_BASE + 0x9024) = (0x239f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);  //0x239f;//0x238f;
            }
        }
        else {
            if (fix_latency) {
                if (burst_length == BURST_LENGTH_128) {
                    REG32(PSRAM_BASE + 0x8024) = (0x089f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:128 ok
                    REG32(PSRAM_BASE + 0x9024) = (0x289f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:128 ok
                }
                else {
                    REG32(PSRAM_BASE + 0x8024) = (0x0b9e & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:2048
                    REG32(PSRAM_BASE + 0x9024) = (0x2b9e & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:2048
                }
            }
            else {
                if (burst_length == BURST_LENGTH_128) {
                    REG32(PSRAM_BASE + 0x8024) = (0x009f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:128 ok
                    REG32(PSRAM_BASE + 0x9024) = (0x209f & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:128 ok
                }
                else {
                    REG32(PSRAM_BASE + 0x8024) = (0x039e & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:2048
                    REG32(PSRAM_BASE + 0x9024) = (0x239e & WBD_MRB_MASK) | (WBD16M_DRIVE << WBD_MRB_OFST);      //burst_length:2048
                }
            }
        }

    }
    else {
        if (fix_latency) {
            REG32(PSRAM_BASE + 0x8024) = (0xb9f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);  //0xb9f; //0xb8f;
            REG32(PSRAM_BASE + 0x9024) = (0x2b9f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST); //0x2b9f;//0x2b8f;
        }
        else {
            REG32(PSRAM_BASE + 0x8024) = (0x39f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);  //0x39f;//0x38f;
            REG32(PSRAM_BASE + 0x9024) = (0x239f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST); //0x239f;//0x238f;
        }
    }
}

static void apm_cfg_seq_table_ap_8m8m(void)
{
    /*[REG_C: SEQ_TABLE]*/ // LUT0: RD 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8090) = 0x8b200400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8094) = 0x9b042004;
    /*[REG_C: SEQ_TABLE]*/ // LUT1: WR 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a0) = 0x8b200480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a4) = 0x97042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT2: RD 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b0) = 0x8b200400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x9b042006;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT3: WR 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x8b200480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x97042006;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h4: WR MR0!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc4c0c4c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x44004400;
    if (fix_latency) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = (0x24024433 & APM_TBL_MASK) | (APM_DRIVE << APM_TBL_OFST); //0x24024433;//0x24024431;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = (0x24024413 & APM_TBL_MASK) | (APM_DRIVE << APM_TBL_OFST); //0x24024413;//0x24024411;
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h6: WR MR4!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xcb2044c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = 0x24094420;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x28022c00;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x2c03;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h5: Global Reset!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x600304ff;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e4) = 0x2820;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h7: WR MR6 halfsleep entry!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0x440044c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0x44f04406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x0;
    /*[REG_C: SEQ_TABLE]*/                                          // LUT 'h8: CEN low halfsleep exit!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x60012880;
    if (is_crane()) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2d606010;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056010; //0x2d056010;
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/// LUT 'h9: RD MR!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x8b200440;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x9f042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'ha: WR MR DATA0!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0xcb2044c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0x24027000;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hb: WR MR DATA1!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8140) = 0xcb2044c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8144) = 0x24027001;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8148) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hc: WR MR8 INCR 1KB!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8150) = 0xcb2044c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8154) = 0x24027002;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8158) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hd: RD MR CADDR!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8160) = 0x8f200440;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8164) = 0x9f042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8168) = 0x0;
}

static void apm_cfg_seq_table_other_ap(void)
{
    /*[REG_C: SEQ_TABLE]*/ // LUT0: RD 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8090) = 0x8b200400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8094) = 0x9b042004;
    /*[REG_C: SEQ_TABLE]*/ // LUT1: WR 125!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a0) = 0x8b200480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a4) = 0x97042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT2: RD 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b0) = 0x8b200400;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x9b042006;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT3: WR 200!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x8b200480;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x97042006;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h4: WR MR0!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0x84c084c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x04000400;
    if (fix_latency) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = (0x24020433 & APM_TBL_MASK) | (APM_DRIVE << APM_TBL_OFST); //0x24020433;//0x24020431;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = (0x24020413 & APM_TBL_MASK) | (APM_DRIVE << APM_TBL_OFST); //0x24020413;//0x24020411;
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h6: WR MR4!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0x8b2004c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = 0x24090420;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x28022c00;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x2c03;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h5: Global Reset!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x200304ff;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e4) = 0x2820;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'h7: WR MR6 halfsleep entry!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0x040004c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0x04f00406;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x0;
    /*[REG_C: SEQ_TABLE]*/                                          // LUT 'h8: CEN low halfsleep exit!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x20012880;
    if (is_crane()) {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2d052010;
    }
    else {
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c052010; //0x2d052010;
    }
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*[REG_C: SEQ_TABLE]*/                                          // LUT 'h9: RD MR!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x8b200440;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x9f042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'ha: WR MR DATA0!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0x8b2004c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0x24023000;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hb: WR MR DATA1!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8140) = 0x8b2004c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8144) = 0x24023001;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8148) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hc: WR MR4 INCR 1KB!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8150) = 0x8b2004c0;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8154) = 0x24023002;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8158) = 0x0;
    /*[REG_C: SEQ_TABLE]*/ // LUT 'hd: RD MR CADDR!
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8160) = 0x8f200440;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8164) = 0x9f042004;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8168) = 0x0;
}

static void apm_cfg_tcph_and_mr_byte(void)
{
    // tcph and MR byte
    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x05000302;
    if (fix_latency) {
        if (is_cranem() ||
            is_cranel() ||
            is_cranels() ||
            is_craneg() ||
            is_cranew()) {
            REG32(PSRAM_BASE + 0x8024) = 0x4029;
            REG32(PSRAM_BASE + 0x9024) = 0x2031;
        }
    }
    else {
        REG32(PSRAM_BASE + 0x8024) = (0x400b & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST); //0x480b;//0x4809;
        REG32(PSRAM_BASE + 0x9024) = (0x2013 & APM_MRB_MASK) | (APM_DRIVE << APM_MRB_OFST); //0x2813;//0x2811;
    }
}

static void apm_cfg_for_engineering_mr2(void)
{
    unsigned mr2 = 0;

    mr2 = psram_rd_mr();
    if (mr2 == ENGINEERING_MR2) {
        // address remap, temp add
        REG32(PSRAM_BASE + 0x8050) = 0x83828180;
        REG32(PSRAM_BASE + 0x8054) = 0x87868584;
        REG32(PSRAM_BASE + 0x8058) = 0x8b8a8988;
        REG32(PSRAM_BASE + 0x805c) = 0x8f8e8d8c;
        REG32(PSRAM_BASE + 0x8060) = 0x93929190;
        if (is_cranem() ||
            is_cranel() ||
            is_cranels() ||
            is_craneg() ||
            is_cranew()) {
            REG32(PSRAM_BASE + 0x8064) = 0x97969594;
            REG32(PSRAM_BASE + 0x8068) = 0x9b9a0098;       // mask out bit 25
        }

        if (is_crane()) {
            REG32(PSRAM_BASE + 0x8064) = 0x00969594;    //0x97969594 mask out bit 25
            REG32(PSRAM_BASE + 0x8068) = 0x9b9a9997;    //0x9b9a0098 mask out bit 25
        }

        REG32(PSRAM_BASE + 0x806c) = 0x9f9e9d9c;

        // /*// SEQ_TABLE*/ LUT0: RD!
        /*[REG_C: SEQ_TABLE]*/ // LUT0: RD 125!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8090) = 0x8f200400;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8094) = 0x9b042004;
        /*[REG_C: SEQ_TABLE]*/ // LUT1: WR 125!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a0) = 0x8f200480;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a4) = 0x97042004;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80a8) = 0x0;
        /*[REG_C: SEQ_TABLE]*/ // LUT2: RD 200!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b0) = 0x8f200400;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x9b042006;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x0;
        /*[REG_C: SEQ_TABLE]*/ // LUT3: WR 200!
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x8f200480;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x97042006;
        /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x0;
    }
}

static void apm_config(unsigned version)
{
    if (version == AP_8M8M) {
        apm_cfg_seq_table_ap_8m8m();
    }
    else {
        apm_cfg_seq_table_other_ap();
    }

    apm_cfg_tcph_and_mr_byte();

    if (ps_slref) {
        REG32(PSRAM_BASE + 0x8024) |= 0x480000;
        REG32(PSRAM_BASE + 0x9024) |= 0x280000;
    }

    apm_cfg_for_engineering_mr2();
}

static void qos_config(void)
{
    unsigned read_data;

    //Set QOS  X1 =4, LTE = 0
    read_data = REG32(0xd4282c00 + 0x118);      //DDRC_PORT_AXI_QOS
    //LogMsg(0, "Before REG32(0xd4282c00+0x118) = 0x%x\n", read_data);
    read_data &= ~((0xf << 27) | (0xf << 24));  // X1 =4, LTE = 0
    read_data |=  (0x4 << 27);
    read_data |=  (0x3 << 24);
    read_data &= ~(0xf << 4);
    read_data |=  (0x2 << 4);
    REG32(0xd4282c00 + 0x118) = read_data;
    //LogMsg(0, "After REG32(0xd4282c00+0x118) = 0x%x\n", read_data);

    //Change lfq_config_cr  qos_2 & qos_3
    read_data = REG32(PSRAM_BASE + 0x400c);
    //LogMsg(0, "Before REG32(PSRAM_BASE+0x400c)= 0x%x\n", read_data);
    read_data &= ~((0x3 << 4) | (0x3 << 6));
    read_data |= ((0x2 << 4) | (0x2 << 6));  // set qos_2 = 3 & qos_3 = 2
    REG32(PSRAM_BASE + 0x400c) = read_data;
    if (is_crane()) {
        // add for in case cr5 or camera stop x1 read lead to latency ultra-long
        REG32(PSRAM_BASE + 0x400c) = 0x446603a0;
    }
    else {
        REG32(PSRAM_BASE + 0x400c) = 0x64660320;
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

static void wait_for_dll_status(unsigned version)
{
    unsigned read_data;

    //wait for dll status
    read_data = REG32(PSRAM_BASE + 0x18010);
    while ((read_data & 0x100) != 0x100) {
        read_data = REG32(PSRAM_BASE + 0x18010);
    }
    if (version == WB_8M8M || version == WB_8M || version == WB_16M) {
        // enable phy for winbond
        read_data = REG32(PSRAM_BASE + 0x18004);
        read_data |= 1 << (24 + 4);
        REG32(PSRAM_BASE + 0x18004) = read_data;
    }
}

static void global_reset(void)
{
    unsigned read_data = 0;

    REG32(PSRAM_BASE + 0x8030) = 0x105;//global reset
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    LOG_INFO("Do Globel Reset\n");
}

static void enable_dfc(void)
{
    #if defined(SDF_ON) || defined(PSRAM_HIGH)
    unsigned read_data = 0;
    #endif

    //enable DFC
    /*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ((0x1 << 24) | 0x1);

    #ifdef SDF_ON
    read_data = REG32(0xd42828b0);      //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
    #endif

    #ifdef PSRAM_HIGH
    read_data = REG32(0xd42828b0);      //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
    #endif
}

static void other_config(unsigned version)
{
    unsigned read_data;

    wait_for_dll_status(version);

    global_reset();

    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    if (version == WB_8M8M || version == WB_8M || version == WB_16M) {
        read_data = REG32(PSRAM_BASE + 0x8000);
        read_data &= 0xff00ffff;
        read_data |= (5 << 16) | (5 << 20);
        REG32(PSRAM_BASE + 0x8000) = read_data;
    }

    enable_dfc();

    if (dis_fast_miss_acc) {
        //disable fast_miss_acc
        read_data = REG32(PSRAM_BASE);
        read_data &= 0xfeff;
        //read_data |= (0x1<<9);
        REG32(PSRAM_BASE) = read_data;
        LOG_INFO("Disable fast_miss_acc\n");
    }
    else {
        LOG_INFO("Enable fast_miss_acc\n");
    }
}

__attribute__ ((unused))
static void psram_init_crane_has_phy(void)
{
    unsigned mr0_addr = 0x0;
    unsigned mr1_addr = 0x1;
    unsigned drive_strength = 0;

    if (is_cranew()) {
        //Set GPIO_16 to func1(psram_rstn)
        unsigned reg = 0;
        reg = read32(0xD401E11C);
        reg &= 0xFFFFFFF8;
        reg |= 0x1;
        write32(0xD401E11C, reg);
    }

    set_config_for_chip();

    /* phy psram */
    if (AP_UHS_8M8M == version) {
        extern void psram_init_uhs_crg(void);
        return psram_init_uhs_crg();
    }

    if (WB_4M == version) {
        extern void psram_init_wb_4MB_cfg(void);
        return psram_init_wb_4MB_cfg();
    }

    if (AP_4M == version) {
        extern void psram_init_ap_4MB_cfg(void);
        return psram_init_ap_4MB_cfg();
    }

    if (version == WB_250MHZ_8M8M) {
        version = WB_8M8M;
    }

    if (version == AP_250MHZ_8M8M || version == WB_XCCELA_8M8M) {
        version = AP_8M8M;
    }

    if (version == WB_250MHZ_8M) {
        version = WB_8M;
    }

    if (version == AP_250MHZ_8M || version == WB_XCCELA_8M) {
        version = AP_8M;
    }

    if (version == AP_250MHZ_16M || version == UNIIC_16M) { //UniIC_16M same as AP_16M
        version = AP_16M;
    }

    if (version == WB_250MHZ_16M) {
        version = WB_16M;
    }

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

    //enable clk
    clk_config();
    //contoller setting
    if (version == WB_8M8M || version == WB_8M || version == WB_16M) {
        if (version == WB_16M) {
            drive_strength = WBD16M_DRIVE;
            LOG_PRINT("[PSRAM] psram init set WB_16M drive_strength = WBD16M_DRIVE_%s .\n", wbd16m_drive_list[drive_strength]);
        }
        else {
            drive_strength = WBD_DRIVE;
            LOG_PRINT("[PSRAM] psram init set WBD drive_strength = WBD_DRIVE_%s .\n", wbd_drive_list[drive_strength]);
        }

        mr0_addr = 0x1000;
        mr1_addr = 0x1002;
        winbond_config();
        winbond_half_sleep();
        winbond_change_freq();

        //mmap cfg
        if (version == WB_8M8M) {
            if (is_cranem()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E684001;
            }
            else if (is_cranel() ||
                     is_cranels() ||
                     is_cranew()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E08400D;
            }
            else {
                REG32(PSRAM_BASE + 0x0004) = 0x7E084001;
            }
        }
        else {
            if (is_cranel() ||
                is_cranels() ||
                is_cranew()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E08000D;
            }
            else {
                REG32(PSRAM_BASE + 0x0004) = 0x7E080001;
            }
        }

    }
    else {
        drive_strength = APM_DRIVE;
        LOG_PRINT("[PSRAM] psram init set APM drive_strength = APM_DRIVE_%s .\n", ap_drive_list[drive_strength]);
        apm_config(version);
        apm_half_sleep();
        apm_change_freq();

        //mmap cfg
        if (version == AP_8M8M) {
            if (is_cranem()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E684001;
            }
            else if (is_cranel() ||
                     is_cranels() ||
                     is_cranew()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E08400D;
            }
            else {
                REG32(PSRAM_BASE + 0x0004) = 0x7E084001;
            }
        }
        else {  //single cs
            if (is_cranel() ||
                is_cranels() ||
                is_cranew()) {
                REG32(PSRAM_BASE + 0x0004) = 0x7E08000D;
            }
            else {
                REG32(PSRAM_BASE + 0x0004) = 0x7E080001;
            }
        }
    }

    LOG_INFO("Mmap psram address spaces: reg[0x%x] = 0x%x\n", PSRAM_BASE + 0x0004, REG32(PSRAM_BASE + 0x0004));
    //qos cfg
    qos_config();
    //cache cfg
    cache_config();
    //other cfg
    other_config(version);
    //phyreg cfg
    phy_reg_config();

    //Add WB64Mb psram refresh
    if (version == WB_8M8M || version == WB_8M) {
        psram_wr_testmode();
        winbond_psram_fast_refresh_entry_test();
    }

    // add test for mr  - - must like this to read mr!!!fisrt read one CS, then read other CS, not let  cross read mr between CS0 and CS1.
    psram_read_mr(mr0_addr, 0);
    psram_read_mr(mr1_addr, 0);

    psram_read_mr(mr0_addr, 1);
    psram_read_mr(mr1_addr, 1);
}

void psram_init_crane(void)
{
    /* Crane common */
    LOG_PRINT("[PSRAM] psram init Crane common.\n");

    chip_id = hw_chip_id();

    //get version information
#ifdef libEFUSE
    version = psram_fuse();
#endif

#if ((AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5)||  \
			(AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5))
    if (true == is_chip_support_auto_detect()) {
        /* eg: craneLR, no phy, auto detect psram type */
        extern void psram_init_crane_no_phy_auto_detect(unsigned efuse_psram_type, char *psram_str);
        psram_init_crane_no_phy_auto_detect(version, &psram_str[0]); //support auto try

        LOG_INFO("########################\n");
        LOG_PRINT("%s\n", psram_str);
        asr_property_set("fuse.psram.type", psram_str);
        LOG_INFO("########################\n");

        return;
    }
#endif


#if ((AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5) && \
			(AROM_VERSION != AROM_VER_2022_11_06_CRANEL_CR5))

    /* psram has phy must use fuse psram type */
    LOG_INFO("########################\n");
    LOG_INFO("Version ID : %d\n", version);
    LOG_PRINT("%s\n", psram_str);
    asr_property_set("fuse.psram.type", psram_str);
    LOG_INFO("########################\n");

    return psram_init_crane_has_phy();
#endif
}
