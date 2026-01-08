/* WB 4MB: enable fast refresh rate */
#include "psram.h"
#include "pmic.h"
#include "cpu.h"
#include <stdio.h>
#include <stdbool.h>

#include "log.h"
#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define REG32(addr) (*(volatile unsigned int *)(addr))
#define writel(v, a) (REG32(a) = (v))
#define readl(a) (REG32(a))
#define PSRAM_BASE  (0xc0100000)

//#define FIX_LATENCY
//#define PSRAM_FPGA_PHY
//#define DIS_PSC_CACHE
#define WINBOND
#define WBD_32Mb
#define INIT_FP1
//#define PSRAM_DUAL_CS

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
    WBD_DRIVE_50ohm = 0,
    WBD_DRIVE_35ohm,
    WBD_DRIVE_100ohm,
    WBD_DRIVE_200ohm,
};

static const char *wbd_drive_list[] = { "50ohm", "35ohm", "100ohm", "200ohm" };
static const char *phy_drive_list[] = { "highZ", "400ohm", "300ohm", "240ohm", "200ohm", "100ohm", "67ohm", "40ohm" };

/* do not change it */
#define PHY_DRIVE   PHY_DRIVE_200ohm
#define WBD_DRIVE   WBD_DRIVE_100ohm

// NO change below
//#define APM_TBL_OFST    0
//#define APM_MRB_OFST    0
#define WBD_TBL_OFST0   4
#define WBD_TBL_OFST1   20
#define WBD_MRB_OFST    4
//#define APM_TBL_MASK    0xfffffffc
//#define APM_MRB_MASK    0xfffffffc
#define WBD_TBL_MASK0   0xffffff8f
#define WBD_TBL_MASK1   0xff8fffff
#define WBD_MRB_MASK    0xffffff8f
int psram_read_mr(unsigned mr_addr, unsigned cs)
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

int psram_write_mr(unsigned mr_addr, unsigned cs, unsigned num)
{
    unsigned read_data = 0;

    /*REG_C, Config PSC*/
    /*fp_mr_data_0[0]*/
    REG32(PSRAM_BASE + 0x8024) = num;

    // write MR register
    REG32(PSRAM_BASE + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE + 0x8030) = 0x10a;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8034), REG32(PSRAM_BASE + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8038), read_data);

    return read_data;
}

static int chip_id = -1;

static bool is_cranem(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return ((CHIP_ID_CRANEM == chip_id) ? true : false);
}

static bool is_craneg(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return ((CHIP_ID_CRANEG == chip_id) ? true : false);
}

static bool is_cranel(void)
{
    if (chip_id < 0) {
        chip_id = hw_chip_id();
    }

    return ((CHIP_ID_CRANEL == chip_id) ? true : false);
}

static void phy_reg_config(void)
{
    unsigned read_data;

    if (is_cranem() || is_cranel() || is_craneg()) {
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

    read_data = REG32(PSRAM_BASE + 0x18004 );
    REG32(PSRAM_BASE + 0x18004 ) = (read_data & 0xfffffff8) | (PHY_DRIVE);      //0x1;
    read_data = REG32(PSRAM_BASE + 0x19004 );
    REG32(PSRAM_BASE + 0x19004 ) = (read_data & 0xfffffff8) | (PHY_DRIVE);      //0x1;
}

int ps_table_dexc_idx = 21;
void winbond_half_lp_mode(void)
{
    //[prog_dfc_table_seq] PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x28
    //[mcUvmRegDfcScdlrSeq] DFC_TB set seq_user_cmd.blk_lfq!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x20;

    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.0: Addr: 8030, Data: 1000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x21;

    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.1: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x22;

    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.2: Addr: 803c, Data: 1, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x23;

    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.3: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x24;

    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 5.4: Addr: 4084, Data: 4, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x83;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;

    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 5.5: Addr: 8008, Data: 83, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence!
    //[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1002;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8034 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;

    //[uc_trig_seq_seq] Write Reg Tb 5.6: Addr: 8034, Data: 1002, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x7!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1107;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x27;

    //[uc_trig_seq_seq] Write Reg Tb 5.7: Addr: 8030, Data: 1107, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x28;

    //[uc_trig_seq_seq] Write Reg Tb 6.0: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x29;

    //[uc_trig_seq_seq] Write Reg Tb 6.1: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x7!
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence done!
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.2: Addr: 8004, Data: 610, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.3: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.4: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 1, EOP: 0, RD=1
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=1, phy_rb=1, rx_bias_rbn=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.5: Addr: 8004, Data: 619, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.6: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 6.7: Addr: 8004, Data: 20, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegWaitDllSeq] DFC_TB wait dll_lock!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x30;

    //[mcUvmRegWaitDllSeq] Write Reg Tb 7.0: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;

    //[mcUvmRegWaitDllSeq] Write Reg Tb 7.1: Addr: 803c, Data: 80000000, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[mcUvmRegPhyOvrdSeq] DFC_TB set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;

    //[mcUvmRegPhyOvrdSeq] Write Reg Tb 7.2: Addr: 8004, Data: 609, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence!
    //[uc_trig_seq_seq] LJ_DEBUG, reset_device=0, write_mr0=0, write_mr=0, read_mr=0!
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence start on index=0x8!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x1108;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;

    //[uc_trig_seq_seq] Write Reg Tb 7.3: Addr: 8030, Data: 1108, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;

    //[uc_trig_seq_seq] Write Reg Tb 7.4: Addr: 8030, Data: 100, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x35;

    //[uc_trig_seq_seq] Write Reg Tb 7.5: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=1
    //[uc_trig_seq_seq] LJ_DEBUG, Trig Sequence finish on index=0x8!
    //[uc_trig_seq_seq] DFC_TB Trigger User Sequence done!
    //[mcUvmRegUpdRWDSSeq] DFC_TB set seq_config_cr.ca_wr_dis=0!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8008 | (1 << ps_table_dexc_idx);
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x36;

    //[mcUvmRegUpdRWDSSeq] Write Reg Tb 7.6: Addr: 8008, Data: 3, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 0, RD=0
    //[mcUvmRegDfcScdlrSeq] DFC_TB clear seq_user_cmd.blk_lfq!
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x37;
    //[mcUvmRegDfcScdlrSeq] Write Reg Tb 7.7: Addr: 8030, Data: 0, REG_WRITE_DISABLE: 0, REQ_PHY: 0, REQ_PMU: 0, EOP: 1, RD=0
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

void psram_init_wb_4MB_cfg(void) // lpddr2_400_init
{
    unsigned read_data;
    //unsigned short read_data_16;
    int i;
    unsigned device_strength = WBD_DRIVE;
    unsigned phy_strength = PHY_DRIVE;

    //unsigned PSRAM_BASE;
    //unsigned PSRAM_DPHY_BASE;
    //unsigned test_num;

    LOG_INFO("psram_init for cranel 2222!!! \n");
    LOG_INFO("device strength = WBD_DRIVE_%s \n", wbd_drive_list[device_strength]);
    LOG_INFO("phy strength = PHY_DRIVE_%s \n", phy_drive_list[phy_strength]);
    //enable psram hclk
    REG32(0xd42828f4) = 0x3;
    REG32(0xd4282800 + 0x15c) &= (~0x40800); //disable pu_dll and pu_phy,set bit11 and bit18 to 0

    //PSRAM_BASE = 0xc0100000;

    REG32(0xd42828b0) |= (1 << 23) | (0x5 << 18); //config for clk

 #ifdef PSRAM_FPGA_PHY
    #ifdef FIX_LATENCY
        #ifdef WINBOND
            #ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xc7ff;               //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xc7ff;               //neg, fix
            #else
    REG32(PSRAM_BASE + 0x18008) = 0x47ff;               //pos, fix
    REG32(PSRAM_BASE + 0x19008) = 0x47ff;               //pos, fix
            #endif
        #else
            #ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xf000;               //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xf000;               //neg, fix
            #else
    REG32(PSRAM_BASE + 0x18008) = 0x7000;               //pos, fix
    REG32(PSRAM_BASE + 0x19008) = 0x7000;               //pos, fix
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
 #endif

    REG32(PSRAM_BASE + 0x18000 ) = 0x1;
    for (i = 0; i < 20; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }
    //for(i=0; i<200; i++) read_data = REG32(PSRAM_BASE + 0x18000) ;
    REG32(PSRAM_BASE + 0x18000 ) = 0x5;

#ifdef WINBOND
    for (i = 0; i < 1000; i++) {
        read_data = REG32(PSRAM_BASE + 0x18000);
    }
// address remap
    REG32(PSRAM_BASE + 0x8078) = 0x87868584;
    REG32(PSRAM_BASE + 0x807c) = 0x8b8a8988;
    REG32(PSRAM_BASE + 0x8080) = 0x8f8e8d8c;
    REG32(PSRAM_BASE + 0x8084) = 0x93929190;
    REG32(PSRAM_BASE + 0x8088) = 0x97969594;
    REG32(PSRAM_BASE + 0x808c) = 0x9b9a9998;
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
#ifdef WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b4) = 0x20048f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b043407;
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80bc) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT3: WR 200!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c0) = 0x93188400;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x20058f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043408;
#ifdef WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c4) = 0x20048f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80c8) = 0x97043407;
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80cc) = 0x0;
#ifdef PSRAM_DUAL_CS
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h4: WR CR0 125M!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc400c460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0xc400c401;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0xc400c400;
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0xc40bc48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0xc403c48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif
#ifdef WBD_32Mb
#ifdef FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40fc48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc40cc48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif  // ~DIS_PSC_CACHE end
#else   // ~FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc407c48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc404c48f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif  // ~DIS_PSC_CACHE end
#endif  //~FIX_LATENCY end
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h6: WR CR0 200M!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0xd318c460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc48fcf10 & WBD_TBL_MASK1) | (WBD_DRIVE << WBD_TBL_OFST1);
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c42b;
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c423;
#endif
#ifdef WBD_32Mb
#ifdef FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c41f;
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c41c;
#endif  // ~DIS_PSC_CACHE end
#else   // ~FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c417;
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c414;
#endif  // ~DIS_PSC_CACHE end
#endif  //~FIX_LATENCY end
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h7: WR MR6 hybird sleep entry!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0xd318c460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc4ffcf10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c4e2;
#ifdef  WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc400cf10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c460;
#endif  // WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'h8: CEN low halfsleep exit!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x62012880;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210;
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

#else // ~PSRAM_DUAL_CS

/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h4: WR CR0 125M!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0x84008460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x84008401;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x84008400;
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840b848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8403848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif
#ifdef WBD_32Mb
#ifdef FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840f848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x840c848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif  // ~DIS_PSC_CACHE end
#else   // ~FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8407848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) =  (0x8404848f & WBD_TBL_MASK0) | (WBD_DRIVE << WBD_TBL_OFST0);
#endif  // ~DIS_PSC_CACHE end
#endif  //~FIX_LATENCY end
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80e0) = 0x2402;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h6: WR CR0 200M!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f0) = 0x93188460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) =  (0x848f8f10 & WBD_TBL_MASK1) | (WBD_DRIVE << WBD_TBL_OFST1);
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402842b;
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028423;
#endif
#ifdef WBD_32Mb
#ifdef FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402841f;
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402841c;
#endif  // ~DIS_PSC_CACHE end
#else   // ~FIX_LATENCY
#ifdef DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028417;
#else   // ~DIS_PSC_CACHE
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x24028414;
#endif  // ~DIS_PSC_CACHE end
#endif  //~FIX_LATENCY end
#endif  // WBD_32Mb end
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80fc) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h7: WR MR6 hybird sleep entry!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8100) = 0x93188460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0x84ff8f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x240284e2;
#ifdef  WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8104) = 0xc400cf10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8108) = 0x2402c460;
#endif  // WBD_32Mb
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;
    LOG_INFO("!!! WB32Mb psram hybrid sleep entry right @@@@  !!!  \n");
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x810c) = 0x0;

/*[REG_C: SEQ_TABLE]*/ // LUT 'h8: CEN low halfsleep exit!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x22012880;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2c056210;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'h9: RD MR!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x931884e0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x20038f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x9f043406;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x812c) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // WBD LUT 'ha: WR MR DATA0!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0x93188460;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0xb0008f10;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x2402b001;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x813c) = 0x0;

#endif // ~PSRAM_DUAL_CS end

// tcph and MR byte
    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x06000302;

#ifdef WBD_32Mb
#ifdef FIX_LATENCY
#ifdef DIS_PSC_CACHE
    REG32(PSRAM_BASE + 0x8024) = (0x0f8f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x1f8f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
#else   // ~DIS_PSC_CACHE
    REG32(PSRAM_BASE + 0x8024) = (0x0c8f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x1c8f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
#endif  // ~DIS_PSC_CACHE end
#else   // ~FIX_LATENCY
#ifdef DIS_PSC_CACHE
    REG32(PSRAM_BASE + 0x8024) = (0x478f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x578f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
#else   // ~DIS_PSC_CACHE
    REG32(PSRAM_BASE + 0x8024) = (0x448f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
    REG32(PSRAM_BASE + 0x9024) = (0x548f & WBD_MRB_MASK) | (WBD_DRIVE << WBD_MRB_OFST);
#endif  // ~DIS_PSC_CACHE end
#endif  // ~FIX_LATENCY end
#endif  // WBD_32Mb end

#else // !WINBOND
#ifndef MMAP_ECO_DONE
// address remap, temp add
    REG32(PSRAM_BASE + 0x8050) = 0x83828180;
    REG32(PSRAM_BASE + 0x8054) = 0x87868584;
    REG32(PSRAM_BASE + 0x8058) = 0x8b8a8988;
    REG32(PSRAM_BASE + 0x805c) = 0x8f8e8d8c;
    REG32(PSRAM_BASE + 0x8060) = 0x93929190;
    REG32(PSRAM_BASE + 0x8064) = 0x97969594;
    REG32(PSRAM_BASE + 0x8068) = 0x9b9a0098; // mask out bit 25
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
#else // MMAP_ECO_DONE
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
#endif // MMAP_ECO_DONE

#ifdef PSRAM_DUAL_CS
/*[REG_C: SEQ_TABLE]*/ // LUT 'h4: WR MR0!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0xc4c0c4c0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x44004400;
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24024431;
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24024411;
#endif
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
/*[REG_C: SEQ_TABLE]*/ // LUT 'h8: CEN low halfsleep exit!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x60012880;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2d606010;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'h9: RD MR!
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
#else                   // ~PSRAM_DUAL_CS
/*[REG_C: SEQ_TABLE]*/  // LUT 'h4: WR MR0!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d0) = 0x84c084c0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d4) = 0x04000400;
#ifdef FIX_LATENCY
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24020431;
#else
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) = 0x24020411;
#endif
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
/*[REG_C: SEQ_TABLE]*/ // LUT 'h8: CEN low halfsleep exit!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8110) = 0x20012880;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8114) = 0x2d602010;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'h9: RD MR!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8120) = 0x8b200440;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8124) = 0x9f042004;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'ha: WR MR DATA0!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8130) = 0x2b2004c0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8134) = 0x24023000;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8138) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'hb: WR MR DATA1!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8140) = 0x8b2004c0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8144) = 0x24023001;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8148) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'hc: WR MR8 INCR 1KB!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8150) = 0x8b2024c0;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8154) = 0x24023002;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8158) = 0x0;
/*[REG_C: SEQ_TABLE]*/ // LUT 'hd: RD MR CADDR!
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8160) = 0x8f200440;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8164) = 0x9f042004;
/*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x8168) = 0x0;

#endif //~PSRAM_DUAL_CS end

// tcph and MR byte
    REG32(PSRAM_BASE + 0x8020) |= 0x03000100;
    REG32(PSRAM_BASE + 0x9020) |= 0x05000302;
    REG32(PSRAM_BASE + 0x8024) = 0x4009;
    REG32(PSRAM_BASE + 0x9024) = 0x2011;
#endif // !WINBOND end

#ifdef WINBOND
    winbond_half_lp_mode();
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
#else //!WINBOND
// /*[prog_dfc_table_seq]*/ PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x20
#ifdef HALF_LP_MODE
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
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x25;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x26;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
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
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
/*[REG_C: DFC_TABLE]*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
#else // ~HALF_LP_MODE
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
#endif // ~HALF_LP_MODE end
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
#endif //!WINBOND

//MMAP
#ifdef PSRAM_DUAL_CS
    REG32(PSRAM_BASE + 0x0004) = 0x7E09400D;
#else
    REG32(PSRAM_BASE + 0x0004) = 0x7E09000D;
#endif


//Set QOS  X1 =4, LTE = 0
    read_data = REG32(0xd4282c00 + 0x118);      //DDRC_PORT_AXI_QOS
    LOG_INFO( "Before REG32(0xd4282c00+0x118) = 0x%x\n", read_data);
    read_data &= ~((0xf << 28) | (0xf << 24));  // X1 =4, LTE = 0
    read_data |=  (0x4 << 28);
    REG32(0xd4282c00 + 0x118) = read_data;
    LOG_INFO( "After REG32(0xd4282c00+0x118) = 0x%x\n", read_data);

//Change lfq_config_cr  qos_2 & qos_3
    read_data = REG32(PSRAM_BASE + 0x400c);
    LOG_INFO( "Before REG32(PSRAM_BASE+0x400c)= 0x%x\n", read_data);
    read_data &= ~((0x3 << 4) | (0x3 << 6));
    read_data |= ((0x3 << 4) | (0x2 << 6)); // set qos_2 = 3 & qos_3 = 2
    REG32(PSRAM_BASE + 0x400c) = read_data;
    LOG_INFO( "After REG32(PSRAM_BASE+0x400c)= 0x%x\n", read_data);


#ifdef DIS_PSC_CACHE
//disable cache
    read_data = REG32(PSRAM_BASE + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(PSRAM_BASE + 0x4000) = read_data;
    LOG_INFO("psram psc cache disable!!!\n");
#else
    LOG_INFO("psram psc cache enable!!!\n");
#endif
    REG32(PSRAM_BASE + 0x4000) |= 0x1;//cache line 128 byte

//
//wait for dll status
    read_data = REG32(PSRAM_BASE + 0x18010);
    while ((read_data & 0x100) != 0x100) {
        read_data = REG32(PSRAM_BASE + 0x18010);
    }

#ifdef WINBOND
    // enable phy for winbond
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x18004) = read_data;
    read_data = REG32(PSRAM_BASE + 0x19004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x19004) = read_data;

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
    REG32(PSRAM_BASE + 0x8030) = 0x105;//global reset
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    LOG_INFO( "Do Globel Reset\n");

    REG32(PSRAM_BASE + 0x8030) = 0x104;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
#endif  // FIX_LATENCY
#endif  // !WINBOND

// lijin: 2018_05_21-18:33 add for swdfc to fp1
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
#else   // ~WINBOND
    REG32(PSRAM_BASE + 0x8034) = 0x0;
    REG32(PSRAM_BASE + 0x8030) = 0x10a;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    REG32(PSRAM_BASE + 0x8034) = 0x4;
    REG32(PSRAM_BASE + 0x8030) = 0x10b;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
#endif  // ~WINBOND end
#endif  // INIT_FP1 end
// lijin: 2018_05_21-18:34 end

//enable DFC
/*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ((0x1 << 24) | 0x1);

#ifdef SDF_ON
    read_data = REG32(0xd42828b0);  //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
#endif
#ifdef PSRAM_HIGH
    read_data = REG32(0xd42828b0);  //dclk fc request
    read_data = read_data & 0xffe3ffc7;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 5) | (1 << 4) | read_data;
#endif

    phy_reg_config();

    psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);

    //write mr0 to add refresh to fast
    wb32Mb_write_cr0();

    psram_read_mr(0x0, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x1000, 0);
    psram_read_mr(0x1002, 0);
}
