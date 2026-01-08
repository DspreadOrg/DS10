#include "psram.h"
#include "efuse.h"
#include "pmic.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "log.h"
#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define PSRAM_REG32(x) *((volatile unsigned int *)(x))
#define REG32(addr) (*(volatile unsigned int *)(addr))
#define writel(v, a) (REG32(a) = (v))
#define readl(a) (REG32(a))

#define PSRAM_BASE  (0xc0100000)

// lijin: 2018_08_16-12:06 add the define for mmap bit 25 issue in Z2 silicon, ECO to make 25 always 0
#define MMAP_ECO_DONE
#define PSRAM_DUAL_CS
#define PS_TABLE_DEXC_IDX 21    // 0 disable
#define PS_TABLE_EEXC_IDX 22    // 1 disable

#define  AUHS
//#define DDR_FPGA_PHY
//#define FIX_LATENCY
//#define DIS_PSC_CACHE

#define MR0 0x0
#define MR2 0x2
#define PSRAM_ADDR_BASE 0x7e000000


enum {
    PHY_DRIVE_highZ = 0,
    PHY_DRIVE_200ohm,
    PHY_DRIVE_100ohm,
    PHY_DRIVE_66ohm,
    PHY_DRIVE_50ohm,
    PHY_DRIVE_40ohm,
    PHY_DRIVE_33ohm,
    PHY_DRIVE_25ohm,
};

enum {
    APM_DRIVE_34ohm = 1, //34.3
    APM_DRIVE_40ohm,
    APM_DRIVE_48ohm,
};

static const char *apm_drive_list[] = { "no_define", "34ohm", "40ohm", "48ohm" };
static const char *phy_drive_list[] = { "highZ", "200ohm", "100ohm", "66ohm", "50ohm", "40ohm", "33ohm", "25ohm" };

#define PHY_DRIVE   PHY_DRIVE_50ohm
#define APM_DRIVE   APM_DRIVE_48ohm


//read mr
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


static void clk_config(void)
{
    //enable psram hclk
    REG32(0xd42828f4) = 0x3;
    REG32(0xd4282800 + 0x15c) |= 0x50800;       //enable pu for dll and phy,and phy_clk rdy

    //PSRAM_BASE = 0xc0100000;

    //enable pll2
#ifdef SDF_ON
    switch (SET_PP_TEST) { //only change tc
        case  PP_SDF_TC:
            REG32(MSG_PORT) = 0xabcd000a;
            REG32(0xd4090000 + 0x120) = (0x80 << 24) | (0x50 << 16) | (0xDD << 8) | 0x67;
            REG32(0xd4090000 + 0x128) = (0x3e << 24) | 0xE27627;
            break;
        case  PP_SDF_WC:
            REG32(MSG_PORT) = 0xabcd000b;
            break;
        default:
            REG32(MSG_PORT) = 0xabcd000f;
            break;
    }
#endif
    REG32(0xd4090000 + 0x124) |= 0x3f;
    REG32(0xd4090000 + 0x128) |= 0x80000000;


    //REG32(0xd42828b0) |= (1<<23)|(0x7<<18);   //config for clk
    //REG32(0xd42828b0) |= (1<<23)|(0x5<<18);   //config for clk
    REG32(0xd42828b0) |= (1 << 23) | (0x0 << 18);     //config for clk
}


#ifdef DDR_FPGA_PHY
static void fpga_phy_config(void)
{
#ifdef FIX_LATENCY
#ifdef WINBOND
#ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xc7ff;   //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xc7ff;   //neg, fix
#else
    REG32(PSRAM_BASE + 0x18008) = 0x47ff;   //pos, fix
    REG32(PSRAM_BASE + 0x19008) = 0x47ff;   //pos, fix
#endif
#else
#ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xf000;   //neg, fix
    REG32(PSRAM_BASE + 0x19008) = 0xf000;   //neg, fix
#else
    LOG_INFO("FIX_LATENCY111 \n");
    //REG32(PSRAM_BASE + 0x18008) = 0x7000; //pos, fix
    //REG32(PSRAM_BASE + 0x19008) = 0x7000; //pos, fix

    REG32(PSRAM_BASE + 0x18008) = 0xf1ff;   //pos, viable
    REG32(PSRAM_BASE + 0x19008) = 0xf1ff;   //pos, viable
    REG32(PSRAM_BASE + 0x1a008) = 0xf1ff;   //pos, viable
    REG32(PSRAM_BASE + 0x1b008) = 0xf1ff;   //pos, viable
#endif
#endif
#else
#ifdef NEG_SAMPLE
    REG32(PSRAM_BASE + 0x18008) = 0xb000;   //neg, viable
    REG32(PSRAM_BASE + 0x19008) = 0xb000;   //neg, viable
#else
    REG32(PSRAM_BASE + 0x18008) = 0x3000;   //pos, viable
    REG32(PSRAM_BASE + 0x19008) = 0x3000;   //pos, viable
#endif
#endif

}
#endif


static void uhs_config(void)
{
    /*REG_C, SEQ_TABLE*/    // APM AUHS Model Enable!
    /*REG_C, Config PSC*/ /*       seq_config_cr*/ REG32(PSRAM_BASE + 0x8008) = 0x43;
    /*REG_C, SEQ_TABLE*/    // AUHS Program RADDR Table!
    /*REG_C, Config PSC*/ /*        raddr_map[1]*/ REG32(PSRAM_BASE + 0x8074) = 0x8e8d8c8b;
    /*REG_C, Config PSC*/ /*        raddr_map[2]*/ REG32(PSRAM_BASE + 0x8078) = 0x9291908f;
    /*REG_C, Config PSC*/ /*        raddr_map[3]*/ REG32(PSRAM_BASE + 0x807c) = 0x96959493;
    /*REG_C, SEQ_TABLE*/ // AUHS Program CADDR Table!
    /*REG_C, Config PSC*/ /*        caddr_map[0]*/ REG32(PSRAM_BASE + 0x8050) = 0x86858483;
    /*REG_C, Config PSC*/ /*        caddr_map[1]*/ REG32(PSRAM_BASE + 0x8054) = 0x8a898887;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h0: RD Cache!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8090) = 0x13100420;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8094) = 0x38000f08;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8098) = 0x9b08;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h1: WR Cache!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a0) = 0x139004a0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a4) = 0x38810f88;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80a8) = 0x20819704;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80ac) = 0x2182;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h2: RD Non-Cache!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b0) = 0x13100400;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b4) = 0x38000f08;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80b8) = 0x9b02;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h3: WR Non-Cache!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c0) = 0x13900480;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c4) = 0x38810f88;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80c8) = 0x20819702;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80cc) = 0x2182;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h5: Global Reset!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e0) = 0x600344ff;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x80e4) = 0x2910;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h7: HalfSleep Entry!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8100) = 0x60034490;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8104) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h8: CEN low HalfSleep Exit!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8110) = 0x73872c06;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8114) = 0x2c057387;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8118) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h9: RD MR!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8120) = 0xb180440;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8124) = 0x9f0a3800;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8128) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'ha: WR MR DATA0!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8130) = 0x440044c0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8134) = 0x4b087000;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8138) = 0x28036001;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x813c) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'hb: WR MR DATA1!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8140) = 0x440044c0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8144) = 0x4b087001;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8148) = 0x28036001;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x814c) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'hc: WR MR8 INCR 1KB!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8150) = 0x440044c0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8154) = 0x4b087002;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8158) = 0x28036001;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x815c) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h10: SelfRefresh Entry!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8190) = 0x20030450;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8194) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h11: SelfRefresh Exit!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81a0) = 0x4d004d0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81a4) = 0x4d004d0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81a8) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h12: AutoRefresh!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81b0) = 0x200304b0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81b4) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h13: ZQCal!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81c0) = 0x440044c0;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81c4) = 0x44054400;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81c8) = 0x28206001;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81cc) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h14: Single CS HalfSleep Entry!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81d0) = 0x20030490;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81d4) = 0x0;
    /*REG_C: SEQ_TABLE*/ // AUHS LUT 'h15: Single CS CEN low HalfSleep Exit!
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81e0) = 0x73872c06;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81e4) = 0x2c057387;
    /*REG_C: SEQ_TABLE*/ REG32(PSRAM_BASE + 0x81e8) = 0x0;
    /*REG_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type LP_T, start tb_index = 0x28
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3800;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x28;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x29;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2a;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2b;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2c;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for HSE_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000010;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x08430 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2d;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0xc;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2e;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0xc;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2f;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for SRE_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8430 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x30;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x31;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x32;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x610;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x33;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x34;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x35;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=1, phy_rb=1, rx_bias_rbn=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x36;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x37;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x38;
    /*REG_C: DFC_TABLE*/ //  wait dll_lock!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x39;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3a;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3b;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for HSX_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000020;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x08430 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0xc;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3d;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3e;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for SRX_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000008;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8430 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3f;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x40;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_EEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x41;

    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=0, dfc_clr_srx=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1000;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x42;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for HSE_REF, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x40;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x43;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500 | (1 << PS_TABLE_DEXC_IDX);
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x44;
    /*REG_C: DFC_TABLE*/    //  clear seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=0, dfc_clr_srx=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x45;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x46;

    /*REG_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type DFC_NCH_T, start tb_index = 0x0
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3800;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x2;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x54084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x4;
    /*REG_C: DFC_TABLE*/    //  clear seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x5;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x6;
    /*REG_C: DFC_TABLE*/    //  PROG_DFC_TABLE for Table Type DFC_CH_T, start tb_index = 0x8
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3800;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x8;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x9;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x1;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xa;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xb;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x4;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x44084;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xc;
    /*REG_C: DFC_TABLE*/ //  set seq_user_cmd.upd_fp=0, upd_fp_wr=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3c00;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xd;
    /*REG_C: DFC_TABLE*/ //  Trigger User Sequence!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8034;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xe;
    /*REG_C: DFC_TABLE*/ //  Trig Sequence start on index=0xa!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3d0a;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0xf;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x100;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x10;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x11;
    /*REG_C: DFC_TABLE*/    //  Trigger User Sequence done!
    /*REG_C: DFC_TABLE*/    //  Trig UHS UC Sequence start for SRE_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8430;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x12;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x13;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x14;
    /*REG_C: DFC_TABLE*/ //  set seq_user_cmd.upd_fp=1, upd_fp_wr=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3a00;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x15;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=1, phy_rb=0, rx_bias_rbn=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x618;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x16;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x17;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x58004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x18;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=1, phy_rb=1, rx_bias_rbn=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x619;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x19;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1a;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1b;
    /*REG_C: DFC_TABLE*/ //  wait dll_lock!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x80000000;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x4803c;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1d;
    /*REG_C: DFC_TABLE*/ //  set phy_ovrd_cr, ovrd_en=0, phy_rb=1, rx_bias_rbn=1!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x609;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8004;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1e;
    /*REG_C: DFC_TABLE*/ //  Trig UHS UC Sequence start for SRX_CMD, cs_all=11, cs=11!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3000008;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x8430;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x1f;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x3;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x20;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x48500;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x21;
    /*REG_C: DFC_TABLE*/    //  clear seq_user_cmd.blk_lfq!
    /*REG_C: DFC_TABLE*/    //  set seq_user_cmd.dfc_mode=0!
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x28030;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x22;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8014) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8018) = 0x0;
    /*REG_C: DFC_TABLE*/ REG32(PSRAM_BASE + 0x8010) = 0x23;


    /*REG_C, Config PSC*/                           // Program FP settings for tgt_fp=0!
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[0]*/  //REG32(PSRAM_BASE + 0x8024) = 0x3517;
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) = (0x351f & 0xffffffc7) | (APM_DRIVE_48ohm << 3);

    /*REG_C, Config PSC*/ /*     fp_mr_data_1[0]*/ REG32(PSRAM_BASE + 0x8028) = 0xd0000000;
    /*REG_C, Config PSC*/ /*     fp_dmy_lat_0[0]*/ REG32(PSRAM_BASE + 0x8424) = 0x105;
    /*REG_C, Config PSC*/ /*   fp_seq_idx_cr1[0]*/ REG32(PSRAM_BASE + 0x8420) = 0x100302;
    /*REG_C, Config PSC*/ /*     fp_rw_timing[0]*/ REG32(PSRAM_BASE + 0x8440) = 0x200c0e04;
    /*REG_C, Config PSC*/ /*fp_uhs_ref_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8480) = 0x812;
    /*REG_C, Config PSC*/ /* fp_uhs_sr_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8484) = 0xb111010;
    /*REG_C, Config PSC*/ /* fp_uhs_hs_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8488) = 0x150014;
    /*REG_C, Config PSC*/ /*      uhs_ref_timing*/ REG32(PSRAM_BASE + 0x8460) = 0x65;
    /*REG_C, Config PSC*/ /*    fp_pra_timing[0]*/ REG32(PSRAM_BASE + 0x8444) = 0x25;
    /*REG_C, Config PSC*/                           //Program phy settings!
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[0]*/  //REG32(PSRAM_BASE + 0x1800c) = 0x8bdc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[0]*/  //REG32(PSRAM_BASE + 0x1800c) = 0x68efc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[0]*/ REG32(PSRAM_BASE + 0x1800c) = 0x67c1c0;


    /*REG_C, Config PSC*/ /*    fp_seq_idx_cr[0]*/ REG32(PSRAM_BASE + 0x8020) = 0x4090100;
    /*REG_C, Config PSC*/                           // Program FP settings for tgt_fp=1!
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[1]*/  //REG32(PSRAM_BASE + 0x9024) = 0x3514;
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[1]*/ REG32(PSRAM_BASE + 0x9024) = (0x351c & 0xffffffc7) | (APM_DRIVE_48ohm << 3);

    /*REG_C, Config PSC*/ /*     fp_mr_data_1[1]*/ REG32(PSRAM_BASE + 0x9028) = 0xd0000000;
    /*REG_C, Config PSC*/ /*     fp_dmy_lat_0[1]*/ REG32(PSRAM_BASE + 0x9424) = 0x610;
    /*REG_C, Config PSC*/ /*   fp_seq_idx_cr1[1]*/ REG32(PSRAM_BASE + 0x9420) = 0x100302;
    /*REG_C, Config PSC*/ /*     fp_rw_timing[1]*/ REG32(PSRAM_BASE + 0x9440) = 0x40200f0d;
    /*REG_C, Config PSC*/ /*fp_uhs_ref_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9480) = 0x1c12;
    /*REG_C, Config PSC*/ /* fp_uhs_sr_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9484) = 0x22113210;
    /*REG_C, Config PSC*/ /* fp_uhs_hs_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9488) = 0x150014;
    /*REG_C, Config PSC*/ /*      uhs_ref_timing*/ REG32(PSRAM_BASE + 0x8460) = 0x65;
    /*REG_C, Config PSC*/ /*    fp_pra_timing[1]*/ REG32(PSRAM_BASE + 0x9444) = 0x49;
    /*REG_C, Config PSC*/                           //Program phy settings!
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[1]*/  //REG32(PSRAM_BASE + 0x1900c) = 0x68bdc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[1]*/  //REG32(PSRAM_BASE + 0x1900c) = 0x68efc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[1]*/ REG32(PSRAM_BASE + 0x1900c) = 0x67c1c0;


    /*REG_C, Config PSC*/ /*    fp_seq_idx_cr[1]*/ REG32(PSRAM_BASE + 0x9020) = 0xb0d0100;
    /*REG_C, Config PSC*/                           // Program FP settings for tgt_fp=2!
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[2]*/  //REG32(PSRAM_BASE + 0xa024) = 0x3511;
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[2]*/ REG32(PSRAM_BASE + 0xa024) = (0x3519 & 0xffffffc7) | (APM_DRIVE_48ohm << 3);

    /*REG_C, Config PSC*/ /*     fp_mr_data_1[2]*/ REG32(PSRAM_BASE + 0xa028) = 0xd0000000;
    /*REG_C, Config PSC*/ /*     fp_dmy_lat_0[2]*/ REG32(PSRAM_BASE + 0xa424) = 0xa19;
    /*REG_C, Config PSC*/ /*   fp_seq_idx_cr1[2]*/ REG32(PSRAM_BASE + 0xa420) = 0x100302;
    /*REG_C, Config PSC*/ /*     fp_rw_timing[2]*/ REG32(PSRAM_BASE + 0xa440) = 0x50300f15;
    /*REG_C, Config PSC*/ /*fp_uhs_ref_idx_cr[2]*/ REG32(PSRAM_BASE + 0xa480) = 0x2c12;
    /*REG_C, Config PSC*/ /* fp_uhs_sr_idx_cr[2]*/ REG32(PSRAM_BASE + 0xa484) = 0x35114c10;
    /*REG_C, Config PSC*/ /* fp_uhs_hs_idx_cr[2]*/ REG32(PSRAM_BASE + 0xa488) = 0x150014;
    /*REG_C, Config PSC*/ /*      uhs_ref_timing*/ REG32(PSRAM_BASE + 0x8460) = 0x65;
    /*REG_C, Config PSC*/ /*    fp_pra_timing[2]*/ REG32(PSRAM_BASE + 0xa444) = 0x38;
    /*REG_C, Config PSC*/                           //Program phy settings!
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[2]*/  //REG32(PSRAM_BASE + 0x1a00c) = 0x68bdc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[2]*/  //REG32(PSRAM_BASE + 0x1a00c) = 0x68efc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[2]*/ REG32(PSRAM_BASE + 0x1a00c) = 0x67c1c0;


    /*REG_C, Config PSC*/ /*    fp_seq_idx_cr[2]*/ REG32(PSRAM_BASE + 0xa020) = 0x10100100;
    /*REG_C, Config PSC*/                           // Program FP settings for tgt_fp=3!
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[3]*/  //REG32(PSRAM_BASE + 0xb024) = 0x3513;
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[3]*/ REG32(PSRAM_BASE + 0xb024) = (0x351b & 0xffffffc7) | (APM_DRIVE_48ohm << 3);

    /*REG_C, Config PSC*/ /*     fp_mr_data_1[3]*/ REG32(PSRAM_BASE + 0xb028) = 0xd0000000;
    /*REG_C, Config PSC*/ /*     fp_dmy_lat_0[3]*/ REG32(PSRAM_BASE + 0xb424) = 0xe21;
    /*REG_C, Config PSC*/ /*   fp_seq_idx_cr1[3]*/ REG32(PSRAM_BASE + 0xb420) = 0x100302;
    /*REG_C, Config PSC*/ /*     fp_rw_timing[3]*/ REG32(PSRAM_BASE + 0xb440) = 0x70400f1d;
    /*REG_C, Config PSC*/ /*fp_uhs_ref_idx_cr[3]*/ REG32(PSRAM_BASE + 0xb480) = 0x3c12;
    /*REG_C, Config PSC*/ /* fp_uhs_sr_idx_cr[3]*/ REG32(PSRAM_BASE + 0xb484) = 0x48116710;
    /*REG_C, Config PSC*/ /* fp_uhs_hs_idx_cr[3]*/ REG32(PSRAM_BASE + 0xb488) = 0x150014;
    /*REG_C, Config PSC*/ /*      uhs_ref_timing*/ REG32(PSRAM_BASE + 0x8460) = 0x65;
    /*REG_C, Config PSC*/ /*    fp_pra_timing[3]*/ REG32(PSRAM_BASE + 0xb444) = 0x49;
    /*REG_C, Config PSC*/                           //Program phy settings!
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[3]*/  //REG32(PSRAM_BASE + 0x1b00c) = 0x68bdc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[3]*/  //REG32(PSRAM_BASE + 0x1b00c) = 0x68efc0;
    /*REG_C, Config PSC*/ /*     aphy_cfg_cr3[3]*/ REG32(PSRAM_BASE + 0x1b00c) = 0x67c1c0;


    /*REG_C, Config PSC*/ /*    fp_seq_idx_cr[3]*/ REG32(PSRAM_BASE + 0xb020) = 0x16130100;

    unsigned dphy_lp_cr = 0;

    dphy_lp_cr = REG32(0xc0114010);
    dphy_lp_cr &= 0xFFFFFFF7;
    REG32(0xc0114010) = dphy_lp_cr;
    LOG_INFO(" reg@[0x%08X]=[0x%08X].\n", 0xc0114010, REG32(0xc0114010));

    REG32(PSRAM_BASE + 0x14010) |= 0x10;

    REG32(0XC0110000 + 0x8008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0x8008) |= 0x0;
    REG32(0XC0110000 + 0x9008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0x9008) |= 0x0;
    REG32(0XC0110000 + 0xa008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0xa008) |= 0x0;
    REG32(0XC0110000 + 0xb008) &= 0xFFFFFF00;
    REG32(0XC0110000 + 0xb008) |= 0x0;

}

static void cache_config(void)
{
    unsigned read_data = 0;

#ifdef DIS_PSC_CACHE
    //disable cache
    read_data = REG32(PSRAM_BASE + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(PSRAM_BASE + 0x4000) = read_data;
    LOG_INFO("cache disabled: @[0x%08X]=[0x%08X] \n", (PSRAM_BASE + 0x4000), REG32(PSRAM_BASE + 0x4000));
#else
    char *cache_size_list[] = { "64B", "128B", "256B", "512B" };
    read_data = REG32(PSRAM_BASE + 0x4000);
    read_data &= ~(0xf << 0);
    read_data |= PSRAM_CACHELINE_64B;
    REG32(PSRAM_BASE + 0x4000) = read_data;
    LOG_INFO("psram cache enabled !!! :cache size=[%s], @[0x%08X]=[0x%08X]\n",
             cache_size_list[PSRAM_CACHELINE_64B], (PSRAM_BASE + 0x4000), REG32(PSRAM_BASE + 0x4000));
#endif
}


static void mmp_config(void)
{
#ifdef PSRAM_16MB_TMP
    REG32(PSRAM_BASE + 0x0004) = 0x7E680001;
#else
#ifdef PSRAM_DUAL_CS
    REG32(PSRAM_BASE + 0x0004) = 0x7E694001;
#else
    REG32(PSRAM_BASE + 0x0004) = 0x7E690001;
    //REG32(PSRAM_BASE + 0x0004) = 0x7E690801;//CS1
#endif
#endif
}

static void other_config(void)
{
    unsigned read_data = 0;

    //wait for dll status
    read_data = REG32(PSRAM_BASE + 0x18010);
    while ((read_data & 0x100) != 0x100) {
        read_data = REG32(PSRAM_BASE + 0x18010);
    }


    // enable PSRAM phy for UHS
    /*REG_C, Config PSC*/   //REG32(PSRAM_BASE + 0x18004) = 0x2600049e;
    /*REG_C, Config PSC*/   //REG32(PSRAM_BASE + 0x18004) = 0xa600049e;
    /*REG_C, Config PSC*/   //REG32(PSRAM_BASE + 0x19004) = 0xa600049a;
    /*REG_C, Config PSC*/   //REG32(PSRAM_BASE + 0x1a004) = 0xa600049e;
    /*REG_C, Config PSC*/   //REG32(PSRAM_BASE + 0x1b004) = 0xa600049e;
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x18004) = (0xa200049c & 0xfffffff8) | PHY_DRIVE_50ohm;
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x19004) = (0xa200049c & 0xfffffff8) | PHY_DRIVE_50ohm;
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x1a004) = (0xa200049c & 0xfffffff8) | PHY_DRIVE_50ohm;
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x1b004) = (0xa200049c & 0xfffffff8) | PHY_DRIVE_50ohm;


    // enable pipe reg for high speed
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x14008) = 0x32;                           //FP0
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x15008) = 0x32;                           //FP1
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x16008) = 0x32;                           //FP2
    /*REG_C, Config PSC*/ REG32(PSRAM_BASE + 0x17008) = 0x32;                           //FP3

    /*REG_C, Config PSC*/ /*       uhs_sr_cfg_cr*/ REG32(PSRAM_BASE + 0x8404) = 0x100;  // TODO, auto_sr time    auto selfrefresh
    /*REG_C, Config PSC*/ /*       uhs_hs_cfg_cr*/ REG32(PSRAM_BASE + 0x8408) = 0x4400;
    /*REG_C, Config PSC*/ /*      uhs_pra_cfg_cr*/ REG32(PSRAM_BASE + 0x840c) = 0x35;   // PRA enable, PRA_PR enable for 128B
    // can set 0x31 for 256B
    // NOTE, must enable after MMAP setting
    /*REG_C, Config PSC*/ /*          uhs_cfg_cr*/ REG32(PSRAM_BASE + 0x8400) = 0x11077f;

    REG32(PSRAM_BASE + 0x4020) = 0x40000200;    //flush_idle_cnt=0x40

    // table entry update
    read_data = REG32(PSRAM_BASE + 0x8000);
    read_data &= 0xff00ffff;
    read_data |= (5 << 16) | (5 << 20);
    REG32(PSRAM_BASE + 0x8000) = read_data;

    // reset device
    REG32(PSRAM_BASE + 0x8030) = 0x105;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    // zqcal
    REG32(PSRAM_BASE + 0x8030) = 0x113;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    // wr mr0 for latency
    REG32(PSRAM_BASE + 0x8034) = 0x0;
    REG32(PSRAM_BASE + 0x8030) = 0x10a;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    // wr mr2 for enable PRA
    REG32(PSRAM_BASE + 0x8034) = 0x2;
    REG32(PSRAM_BASE + 0x8030) = 0x10b;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }


    //enable DFC
    /*SEQ_TABLE*/ REG32(PSRAM_BASE + 0x8000) |= ((0x1 << 24) | 0x1);

#ifdef DDR_FPGA_PHY
#else
#ifdef SDF_ON
#ifdef AUHS
    read_data = REG32(0xd42828b0);  //dclk fc request
    read_data = read_data & 0xffe3ff87;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x4 << 18) | (1 << 6) | (3 << 4) | read_data;
#else
    read_data = REG32(0xd42828b0);  //dclk fc request
    read_data = read_data & 0xffe3ff87;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 26) | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
#endif
#else
#define PSRAM_HIGH
#ifdef PSRAM_HIGH
    read_data = REG32(0xd42828b0);  //dclk fc request
    read_data = read_data & 0xffe3ff87;
    REG32(0xd42828b0) = 0x01000000 | (0x1 << 18) | (1 << 6) | (1 << 4) | read_data;
#endif
#endif
#endif

    //analogy phy rx dq delay
    REG32(0xc0110000 + 0x8070) = 0x08080808;
    REG32(0xc0110000 + 0x9070) = 0x08080808;
    REG32(0xc0110000 + 0xa070) = 0x08080808;
    REG32(0xc0110000 + 0xb070) = 0x08080808;

    REG32(0xc0110000 + 0x8074) = 0x08080808;
    REG32(0xc0110000 + 0x9074) = 0x08080808;
    REG32(0xc0110000 + 0xa074) = 0x08080808;
    REG32(0xc0110000 + 0xb074) = 0x08080808;

}

void psram_init_uhs_crg(void)
{
    unsigned mr_addr_list[] = { 0x0, 0x1, 0x2,  0x4, 0x5, 0x6, 0x20, 0x28 };
    unsigned inx = 0;
    unsigned device_strength = APM_DRIVE;
    unsigned phy_strength = PHY_DRIVE;

    LOG_INFO("[PSRAM] psram init uhs crg, CraneG .\n");
    LOG_INFO("device strength = APM_DRIVE_%s.\n", apm_drive_list[device_strength]);
    LOG_INFO("phy strength = PHY_DRIVE_%s.\n", phy_drive_list[phy_strength]);
    /* cr5 should not use uhs psram, or cr5 will invoked pmic interface. Only mcu can invoke pmic interface */
    /*uhs must enable ldo11 1.2v, ldo6 is not used*/
    pmic_set_uhs_voltage();

    clk_config();

#ifdef DDR_FPGA_PHY
    fpga_phy_config();
#endif

    uhs_config();

    //MMAP
    mmp_config();


#ifdef AUHS
    REG32(PSRAM_BASE + 0x0004) |= 0x2;// enable UHS_PSRAM_EN
#endif

    cache_config();

    other_config();

    REG32(0xc0100000 + 0x8408) = 0xe400;
    LOG_PRINT("0xc0108408:[0x%x]!!!\n", REG32(0xc0100000 + 0x8408));

    for (inx = 0; inx < sizeof(mr_addr_list) / sizeof(unsigned); inx++) {
        psram_read_mr(mr_addr_list[inx], 0);
        psram_read_mr(mr_addr_list[inx], 1);
    }
}
