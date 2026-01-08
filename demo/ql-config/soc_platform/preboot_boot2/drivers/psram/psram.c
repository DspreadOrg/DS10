#include "psram.h"
#include "reg.h"
#include "cpu.h"
#include "syscall-arom.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "property.h"
#include "psram_mem_verify.h"

#include "log.h"
#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define PSRAM_REG32(x) *((volatile unsigned int *)(x))

#define APB_SPARE           (0xd4090000)
#define PLL1_SW_CTRL2       (APB_SPARE + 0x104)
#define PLL1_SW_CTRL12      (APB_SPARE + 0x12C) //craneLR

#define MCU_AON_BASE         0x41000000
#define SYS_PLL1_CTL1_REG    (MCU_AON_BASE + 0x80)  /* PLL1 sw control reg for cr5 */
#define SYS_PLL1_CTL3_REG    (MCU_AON_BASE + 0x88)  /* PLL1 sw control reg for cr5 */

#define PMU_MC_HW_SLP_TYPE  (PMU_BASE + 0x0b0)

static void psram_cranew_pll1_enable(void);

#if PSRAM_MEM_TEST_ALL
static void psram_update_freq_as_boot33(const char *);
#endif

#ifdef BOARD_CRANE
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
static unsigned int psram_get_min_freq(void)
{
    unsigned phy_clk_level;

    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            phy_clk_level = PSRAM_FREQ_312M;
            break;
        default:
            phy_clk_level = PSRAM_FREQ_156M;
            break;
    }

    return phy_clk_level;
}
#endif
#endif

static unsigned int psram_get_max_freq(void)
{
    unsigned phy_clk_level;

    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEW:
        case CHIP_ID_FULMAR:
            //mcu and cr5 z2: 416Mbps, 208MHz
            phy_clk_level = PSRAM_FREQ_416M;
            break;

        case CHIP_ID_WATCH1:
            if (hw_rev_id() == REV_ID_WATCH1_A0) {
                //watch1 A0: 500Mbps, 250MHZ
                phy_clk_level = PSRAM_FREQ_500M;
            }
            else {
                //watch1 Z1: 416Mbps, 208MHZ
                phy_clk_level = PSRAM_FREQ_416M;
            }

            break;

        default:
            /* AP_4M not support 416M, WB_4M not support 416M, preboot use stable LV2:350M */
            phy_clk_level = PSRAM_FREQ_350M;
            break;
    }

    return phy_clk_level;
}

/*
 * haps FB1 PLL_1 GCLK0: 100MHz, max freq 20MHz,
 * so it cannot change freq for psram, if change, write psram will hung.
 * it also cannot change freq for qspi, 13MHz, will be OK.
 *
 * PZ1 can test change freq for psram and qspi
 */
/*
 * PZ1, Z1:
 * MCU: no phy, x8 bits, WB_32M, psram size= 16MBytes, psram clk 120MHz, MCU_SYS_CLK_CTRL: 240MHz
 * CR5: has phy, max 416MHz

 * PZ1, Z2:
 * MCU: no phy, x16 bits, AP8M_8M, psram size= 16MBytes, psram clk : 208MHz
 * CR5: no phy, 245MHz
 */
void psram_init(void)
{
#if AROM_VERSION != AROM_VER_2021_10_16_CRANEL_A0 //cranel裁剪代码，加快开机时间 quectel
    /* haps: cannot test change freq, it will hung, please test on PZ1*/
    if (hw_platform_type() == HW_PLATFORM_TYPE_FPGA) {
#ifdef BOARD_CRANE
#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
        extern void psram_init_fpga_cr5(void);
        psram_init_fpga_cr5();
#else
        extern void psram_init_fpga_no_phy(void);
        psram_init_fpga_no_phy();
#endif
#endif

#ifdef BOARD_CRANEW_MCU
        extern void psram_init_fpga_mcu(void);
        psram_init_fpga_mcu();
#endif
        return;
    }
    /* PZ1 */
    else if (hw_platform_type() == HW_PLATFORM_TYPE_ZEBU_Z1) {
#ifdef BOARD_CRANEW_MCU
        extern void psram_init_zebu_z1_mcu(void);
        psram_init_zebu_z1_mcu();
        psram_phy_fc_config(psram_get_max_freq());
        //psram_cranew_mcu_fc_config(PSRAM_FREQ_312M); //update 104MHz, 208Mbps for test
        //psram_cranew_mcu_fc_config(PSRAM_FREQ_416M); //update 208MHz for test
#else
#if AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5
        extern void psram_init_zebu_z1_cr5(void);
        psram_init_zebu_z1_cr5();
        psram_phy_fc_config(psram_get_max_freq());
#else
        extern void psram_init_zebu_no_phy(void);
        psram_init_zebu_no_phy();
#endif
#endif
        return;
    }

    /* EVB */
#endif //quectel
#ifdef BOARD_CRANEW_MCU
    if (CHIP_ID_CRANEW == hw_chip_id() ||
        CHIP_ID_FULMAR == hw_chip_id() ||
        CHIP_ID_WATCH1 == hw_chip_id()) {
        extern void psram_init_cranew_mcu(void);
        psram_init_cranew_mcu();
    }
#else
    psram_pll1_enable();

    /*
     * crane: must set freq min, or psram hung.
     * craneL: psram AP_4M workaround for preboot twice from high freq.
     * if do try download, work around method 1 : the second preboot set freq to min before pram init, recover psram device to low freq, or psram will hang
     * new chips set min freq in its psram driver.
     */
#if AROM_VERSION != AROM_VER_2022_08_08_CRANEW_CR5
    if (CHIP_ID_CRANE == hw_chip_id()) {
        /* crane force upload, the second preboot must set 156M min freq before psram init */
        psram_phy_fc_config(psram_get_min_freq());
    }
#endif

    extern void psram_init_crane(void);
    psram_init_crane();
#endif

    /* craneLR/craneL: AP_4M try download fail, preboot should never update freq, Only psram init freq once, eg: 1602 craneLR psram init freq 227Mbps < 266Mbps is low freq. */
    const char *prop_str;
    prop_str = asr_property_get("fuse.psram.type");
    if (NULL == prop_str) {
        LOG_PRINT("get prop fuse.psram.type failed!\n");
        return;
    }

    if (0 != strcmp(prop_str, "AP_4M")) {
        psram_phy_fc_config(psram_get_max_freq());
    }

#if PSRAM_MEM_TEST_ALL
    psram_update_freq_as_boot33(prop_str);
#endif
}

/* cranew mcu qspi clk use osc instead of PLL1,
 * if mcu psram freq pclk use pll1, need enable PLL1, currently use PSRAM_MCU_PCCLK_208_FROM_OSC2
 */
static void psram_cranew_pll1_enable(void)
{
    /*
     * cranew CR5, change PLL enable functions from [APB_spare2_reg] to [MCU_AON: sys_pll1_ctlx_reg],
     * CR5 bootrom enabled PLL1_DIV23_EN for qspi which need 108MHz src clk to get 13MHZ, the clock not used by psram used by qspi
     * CR5 qspi freq if changed to other PLL, Refer to PMU_QSPI_CLK_RES_CTRL(PMU_BASE + 0x60), QSPI_CLK_SEL bit[8:6] to check PLL
     * so enable all pll1 for qspi and psram here,
     * watch sw can confirm which pll1 can be disabled if no use.
     */
    /*
       PLL1_SW_CTRL2                       (APB_SPARE + 0x104)
       PLL1_307_EN                         (1<<5) //default is 1
       PLL1_350_EN                         (1<<4) //default is 1
       PLL1_409_EN                         (1<<3) //default is 1
       PLL1_491_EN                         (1<<2) //default is 1
       PLL1_614_EN                         (1<<1) //default is 1
       PLL1_819_EN                         (1<<0) //default is 0

       SYS_PLL1_CTL1_REG                   (MCU_AON_BASE + 0x80), QSPI_CLK_SEL
       PLL1_DIV8_EN                        (1<<5) //default is 1, 312MHz
       PLL1_DIV7_EN                        (1<<4) //default is 1, 356MHz
       PLL1_DIV6_EN                        (1<<3) //default is 1, 416MHz
       PLL1_DIV5_EN                        (1<<2) //default is 1, 499MHz
       PLL1_DIV4_EN                        (1<<1) //default is 1
       PLL1_DIV3_EN                        (1<<0) //default is 0

       SYS_PLL1_CTL3_REG                   (MCU_AON_BASE + 0x88)
       PLL1_DIV23_EN                       (1<<15) //default is 0, 108MHz
       PLL1_DIV13_EN                       (1<<14) //default is 0, 192MHz
       PLL1_DIV11_EN                       (1<<13) //default is 0, 227MHz
     */
    //cr5 z2
    PSRAM_REG32(PLL1_SW_CTRL2) |= 0X3F;
    PSRAM_REG32(SYS_PLL1_CTL1_REG) |= 0x3F;         //bit[5:0]
    PSRAM_REG32(SYS_PLL1_CTL3_REG) |= (0x7 << 13);  //bit[2:0]
}

/*
 * APB_spare2_reg: PLL1 SW control Register2
 * Craneg Z1 need manually enable PLL1, Z2  PLL1 is already enabled by default.
 */
void psram_pll1_enable(void)
{
    if (CHIP_ID_CRANEW == hw_chip_id() ||
        CHIP_ID_FULMAR == hw_chip_id() ||
        CHIP_ID_WATCH1 == hw_chip_id()) {
        psram_cranew_pll1_enable();
    }
    else if (CHIP_ID_CRANELR == hw_chip_id() ||
             CHIP_ID_CRANELG == hw_chip_id() ||
             CHIP_ID_CRANELRH == hw_chip_id() ||
             CHIP_ID_CRANELS == hw_chip_id() ||
             CHIP_ID_CRANELB == hw_chip_id()) {
        /*
           craneLR: APB_spare2_reg and APB_spare12_reg
           PLL1_SW_CTRL2       (APB_SPARE + 0x104)
           PLL1_DIV8_EN        (1<<5) //default is 1
           PLL1_DIV7_EN        (1<<4) //default is 1
           PLL1_DIV6_EN        (1<<3) //default is 1
           PLL1_DIV5_EN        (1<<2) //default is 1
           PLL1_DIV4_EN        (1<<1) //default is 1
           PLL1_DIV3_EN        (1<<0) //default is 0

           PLL1_SW_CTRL12      (APB_SPARE + 0x12C)
           PLL1_DIV23_EN       (1<<2) //default is 0
           PLL1_DIV13_EN       (1<<1) //default is 0
           PLL1_DIV11_EN       (1<<0) //default is 0
         */
        PSRAM_REG32(PLL1_SW_CTRL2) |= 0x3F; //bit[5:0]
        PSRAM_REG32(PLL1_SW_CTRL12) |= 0x7; //bit[2:0]
    }
    else {
        //craneg
        PSRAM_REG32(PLL1_SW_CTRL2) |= 0x3F;
    }
}

/*********************************************************************/
/* PCCLK Clk for PHY CLK Select :
 * 0x0 = PLL1 312 MHz
 * 0x1 = PLL1 350 MHz
 * 0x2 = PLL1 416 MHz
 * 0x5 = PLL1 156 MHz

 * PHY Clk Divider:
 * PHY Clk = PHY_SEL / (this field +1)
 *
 * [PMU_MC_HW_SLP_TYPE(0xb0)]  bit 20~18 phy clk select

 * Bit 5 means phyclk fc eanble.
 * If Bit5 ==0, FC only avaiable for pcclk;
 * If Bit5 ==1, FC avaiable for both pcclk and phyclk.
 * Bit 4 means FC points.
 * Bit 3 means FC/LP flag. 0==lp, 1== fc. Bit 3 has HW ctrl, don't need SW change
 *
 * Normal mode:
 * Crane and CraneG_A0(3603s) max to LV3 PLL1_416;
 * Crane min to LV0 PLL1_156;
 * CraneG_A0(3603s) min to LV1 PLL1_312;
 *
 * MCU
 * MCG_AON_REG [PMU_MC_HW_SLP_TYPE(0xb0)] MPSC_CLK_SEL(clksel): 0:208, 1:300, 2:499, 3:624, 4:250, 5:125
 */
static const struct PsramPP_t PsramPP_crane[] =
{
#ifdef BOARD_CRANEW_MCU
    /* z2: PMU_MPSC_CLK_CTRL: MPSC_CLK_SEL[19:18], is pc clk, no phy */
    /*LV0:   75MHz*/ { 75,  0x1, 1, CLK_MODE_2X },      // PLL1_300 -- 300/2/2 clock --  75MHz  150Mbps
    /*LV1:  104MHz*/ { 104, 0x0, 1, CLK_MODE_1X },      // PLL1_208 -- 208/2/1 clock -- 104MHz  208Mbps
    /*LV2:  150MHz*/ { 150, 0x1, 1, CLK_MODE_1X },      // PLL1_300 -- 300/2/1 clock -- 150MHz  300Mbps
    /*LV3:  208MHz*/ { 208, 0x0, 0, CLK_MODE_1X },      // PLL1_208 -- 208/1/1 clock -- 208MHz  416Mbps
    /*LV4:  250MHZ*/ { 250, 0x4, 0, CLK_MODE_1X },      // OSC3_250 -- 250/1/1 clock -- 250MHz  500Mbps
#elif (AROM_VERSION == AROM_VER_2022_08_08_CRANEW_CR5 || AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5)
    /* craneg/craenw cr5 z2: pmu_mv_he_slp_type: PHY_CLK_SEL[20:18], */
    /*LV0:     78M*/ { 78,  0x0, 1, CLK_MODE_2X },      // PLL1_312 -312/2/2 clock -- 78MHz
    /*LV1:  156MHz*/ { 156, 0x0, 1, CLK_MODE_1X },      // PLL1_312 -- 312/2 clock -- 156MHz
    /*LV2:  178MHz*/ { 178, 0x1, 1, CLK_MODE_1X },      // PLL1_356 -- 356/2 clock -- 178MHz
    /*LV3:  208MHz*/ { 208, 0x2, 1, CLK_MODE_1X },      // PLL1_416 -- 416/2 clock -- 208MHz   416Mbps

#if 0
    /* craneLR can use for test lower freq */
    /*LV4:  192MHz*/ { 192, 0x4, 0, CLK_MODE_1X },      // PLL1_192 -- 192/1 clock -- 192MHz   384Mbps
    /*LV5:  227MHz*/ { 227, 0x5, 0, CLK_MODE_1X },      // PLL1_227 -- 227/1 clock -- 227MHz   454Mbps
#endif
#else
    /* craneg not support clk mode, bit8 default is 1, can not set 0; CLK_MODE_MAX means not support CLK_MODDE */
    /*LV0:     78M*/ { 78,  0x5, 1, CLK_MODE_MAX },     // PLL1_156 -- 156/2 clock -- 78MHz
    /*LV1:  156MHz*/ { 156, 0x0, 1, CLK_MODE_MAX },     // PLL1_312 -- 312/2 clock -- 156MHz
    /*LV2:  175MHz*/ { 175, 0x1, 1, CLK_MODE_MAX },     // PLL1_350 -- 350/2 clock -- 175MHz
    /*LV3:  208MHz*/ { 208, 0x2, 1, CLK_MODE_MAX },     // PLL1_416 -- 416/2 clock -- 208MHz   416Mbps
#endif
};

/*
 * Crane, CraneM, craneL, craneLS A0(support clkMode)
 * phyclk_level/dclk_level, bit[20:18]: min:            LV0, dclk_level=0(78M,  PLL1_156);
 *                           max in preboot: LV2, dclk_level=2(175M, PLL1_350);
 * pcclk_sel,    bit[28:26]: always                          0(      PLL1_312);
 */
static void psram_phy_fc_config_default(unsigned dclk_level, unsigned pcclk_sel)
{
    unsigned reg_val = 0;
    unsigned phyclk_fc_en = 1;
    unsigned phyclk_sel = 0;
    unsigned phyclk_div = 0;
    clk_mode_t clk_mode;
    unsigned high_fp = 0;

    //phy clk is dividered
    LOG_INFO("PSRAM PHY frequency changed to %d\n", PsramPP_crane[dclk_level].freq);

    phyclk_sel = PsramPP_crane[dclk_level].clkSel;
    phyclk_div = PsramPP_crane[dclk_level].clkDiv;
    clk_mode = PsramPP_crane[dclk_level].clkMode;

    if (dclk_level >= 1) {
        high_fp = 1;
    }

    if (CLK_MODE_MAX != clk_mode) {
        //eg: craneLS A0 add clk_mode
        reg_val &= ~(1 << 8);
        reg_val |= (clk_mode << 8);
    }

#if 0
    LOG_INFO("Before: PMU_MC_HW_SLP_TYPE, [0x%08X]=[0x%08X].\n",
             (unsigned int)PMU_MC_HW_SLP_TYPE,
             (unsigned int)PSRAM_REG32(PMU_MC_HW_SLP_TYPE)); /* clk freq select bit20~18 */
    LOG_INFO("AFter:");
#endif
    reg_val = PSRAM_REG32(PMU_MC_HW_SLP_TYPE);
    reg_val &= ~((0x7 << 26) | (0x7 << 18) | (0x3 << 16) | (0x3 << 4)); // don't touch bit3, HW will automatically switch b/w LP and FC.
    reg_val |= (pcclk_sel << 26) | (phyclk_sel << 18) | (phyclk_div << 16) | (phyclk_fc_en << 5) | (high_fp << 4);
    reg_val |= (1 << 24);                                               // PCCLK_AND_PHYCLK Frequency Change Request

    if (hw_platform_type() == HW_PLATFORM_TYPE_SILICON) {
        PSRAM_REG32(PMU_MC_HW_SLP_TYPE) = reg_val;
        while (PSRAM_REG32(PMU_MC_HW_SLP_TYPE) & (1 << 24)) {}
    }

    //LOG_INFO("pcclk_sel=%d, phyclk_sel=%d, phyclk_div=%d, clk_mode=%d\n", pcclk_sel, dclk_level, phyclk_div, clk_mode);
    LOG_INFO("PMU_MC_HW_SLP_TYPE, [0x%08X]=[0x%08X].\n",
             (unsigned int)PMU_MC_HW_SLP_TYPE,
             (unsigned int)PSRAM_REG32(PMU_MC_HW_SLP_TYPE)); /* clk freq select bit20~18 */
}

/*
 * CraneG_A0, cranew cr5 Z1, cranew CR5 Z2(support clkMode)
 * phyclk_level, bit[20:18]: min:            LV1, dclk_level=1(156M, PLL1_312);
 *                           max in preboot: LV3, dclk_level=3(208M, PLL1_416);
 * pcclk_sel,    bit[28:26]: always                          1(      PLL1_350);
 */
static void psram_phy_fc_config_craneg_a0(unsigned phyclk_level, unsigned pcclk_sel)
{
    unsigned reg_val = 0;
    //unsigned pcclk_sel = 1; // psc controller clock: 350MHz // PCCLK Clk Select<p>0x0 = PLL1 312 MHz<p>0x1 = PLL1 350 MHz<p>0x2 = PLL1 416 MHz<p>0x3 = 499MHz<p>0x4 = PLL1 208 MHz<p>0x5 = PLL1 156 MHz<p>0x6 & 0x7 = reserved
    unsigned phyclk_sel;
    unsigned phyclk_div;
    clk_mode_t clk_mode;
    unsigned fp_num = 1; // NON UHS: 2 FP; UHS: 4 FP

    //phy clk is dividered
    LOG_INFO("PSRAM PHY frequency changed to %d\n", PsramPP_crane[phyclk_level].freq);

    phyclk_sel = PsramPP_crane[phyclk_level].clkSel;
    phyclk_div = PsramPP_crane[phyclk_level].clkDiv;
    clk_mode = PsramPP_crane[phyclk_level].clkMode;

    reg_val = PSRAM_REG32(PMU_MC_HW_SLP_TYPE);
    reg_val &= ~((0x7 << 26) | (0x7 << 18) | (0x3 << 16) | (0xf << 3)); // don't touch bit3, HW will automatically switch b/w LP and FC.

    if (CLK_MODE_MAX != clk_mode) {
        //eg: cranew cr5 add clk_mode
        reg_val &= ~(1 << 8);
        reg_val |= (clk_mode << 8);
    }

    reg_val |=  (pcclk_sel << 26) | (phyclk_sel << 18) | (phyclk_div << 16);
    reg_val |= (1 << 6) | (fp_num << 4);                                // bit6: phy & psc clk fc enable; bit[4:3]: fp_num, mc_fc_type Bit[6:3]
    reg_val |= (1 << 24);                                               // PCCLK_AND_PHYCLK Frequency Change Request

    if (hw_platform_type() == HW_PLATFORM_TYPE_SILICON) {
        PSRAM_REG32(PMU_MC_HW_SLP_TYPE) = reg_val;
        while (PSRAM_REG32(PMU_MC_HW_SLP_TYPE) & (1 << 24)) {}
    }

    LOG_INFO("pcclk_sel=%d, phyclk_sel=%d, phyclk_div=%d, clk_mode=%d\n", pcclk_sel, phyclk_sel, phyclk_div, clk_mode);
    LOG_INFO("PMU_MC_HW_SLP_TYPE, [0x%08X]=[0x%08X].\n",
             (unsigned int)PMU_MC_HW_SLP_TYPE,
             (unsigned int)PSRAM_REG32(PMU_MC_HW_SLP_TYPE)); /* clk freq select bit20~18 */
}

#ifdef BOARD_CRANEW_MCU
/* MCU_AON: PMU_MPSC_CLK_CTRL */
/*
 * Z2
 * cranew mcu/cr5 no phy,
 * cranew MCU:
 * z1:  8x, 120MHz, not support to dynamic change freq.
 * Z2: 16x, 208MHz, support to dynamic change freq.
 *
 * cranew mcu osc only 208 and 300MHZ two choose,
 * cranew mcu qspi freq must osc freq(cannot use pll1 freq), mcu psram alse can use osc freq or PLL1(if use PLL1, must enable PLL1)
 * cranew cr5 qspi freq ue pll1 freq.
 *
 * dclk_level = pcclk_sel/divider/clk_mode;
 * eg:  208M/1(dividor)/2(2xmode) = 104MHz
 *      208M/2(dividor)/1(1xmode) = 104MHz
 */
static void psram_cranew_mcu_fc_config(unsigned pscclk_level)
{
    unsigned int reg_val = 0;
    unsigned pscclk_sel;
    unsigned pscclk_div;
    clk_mode_t clk_mode;

    //phy clk is dividered
    LOG_INFO("MCU PSRAM PHY frequency changed to %d\n", PsramPP_crane[pscclk_level].freq);

    pscclk_sel = PsramPP_crane[pscclk_level].clkSel;
    pscclk_div = PsramPP_crane[pscclk_level].clkDiv;
    clk_mode = PsramPP_crane[pscclk_level].clkMode;
    /* mcu psram init use fc_req, if update freq again, please use dynamic dfc_req */
    unsigned mpsc_clk_fc_req = 24;

    if (hw_chip_id() == CHIP_ID_WATCH1 && hw_rev_id() == REV_ID_WATCH1_A0) {
        mpsc_clk_fc_req = 25;
        reg_val = PSRAM_REG32(PMU_MPSC_CLK_CTRL);
        reg_val &= ~(0x3f << 16);   /* Bit21~16 clear */
        reg_val |= (pscclk_div << 16) | (pscclk_sel << 18) | (clk_mode << 21);
        reg_val |= (1 << mpsc_clk_fc_req);       /* send freg change request */
        PSRAM_REG32(PMU_MPSC_CLK_CTRL) = reg_val;
        /* wait for freg change over */
        while (PSRAM_REG32(PMU_MPSC_CLK_CTRL) & (1 << mpsc_clk_fc_req)) {}
    }
    else {
        reg_val = PSRAM_REG32(PMU_MPSC_CLK_CTRL);
        reg_val &= ~(0x1f << 16);   /* Bit20~16 clear */
        reg_val |= (pscclk_div << 16) | (pscclk_sel << 18) | (clk_mode << 20);
        reg_val |= (1 << mpsc_clk_fc_req);       /* send freg change request */
        PSRAM_REG32(PMU_MPSC_CLK_CTRL) = reg_val;
        /* wait for freg change over */
        while (PSRAM_REG32(PMU_MPSC_CLK_CTRL) & (1 << mpsc_clk_fc_req)) {}
    }
    //LOG_INFO("pscclk_sel=%d, pscclk_div=%d, clk_mode=%d\n", pscclk_sel, pscclk_div, clk_mode);
    LOG_INFO("MCU: PMU_MPSC_CLK_CTRL, [0x%08X]=[0x%08X].\n",
             (unsigned int)PMU_MPSC_CLK_CTRL,
             (unsigned int)PSRAM_REG32(PMU_MPSC_CLK_CTRL)); /* clk freq select bit19~18 */
}
#endif

void psram_phy_fc_config(unsigned phyclk_level)
{
    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            psram_phy_fc_config_craneg_a0(phyclk_level, PSRAM_PCCLK_PLL1_350M);
            break;

        case CHIP_ID_CRANEW:
        case CHIP_ID_FULMAR:
        case CHIP_ID_WATCH1:
#ifdef BOARD_CRANEW_MCU
            psram_cranew_mcu_fc_config(phyclk_level);                           //cranew Z2, fulmar Z1
#else
            psram_phy_fc_config_craneg_a0(phyclk_level, PSRAM_PCCLK_PLL1_350M); //z2
#endif
            break;

        case CHIP_ID_CRANE:
        case CHIP_ID_CRANEM:
        case CHIP_ID_CRANEL:
            psram_phy_fc_config_default(phyclk_level, PSRAM_PCCLK_PLL1_312M); /* old chips use history */
            break;

        default:
            /* in future chip, all use PSRAM_PCCLK_PLL1_350M */
            psram_phy_fc_config_default(phyclk_level, PSRAM_PCCLK_PLL1_350M);
            break;
    }
}

#if PSRAM_MEM_TEST_ALL
/* pem_mem_verify.c simulat the freq of boot33 */
static void psram_update_freq_as_boot33(const char *prop_str)
{
    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            psram_phy_fc_config_craneg_a0(PSRAM_FREQ_416M, PSRAM_PCCLK_PLL1_350M);
            break;

        case CHIP_ID_CRANEW:
        case CHIP_ID_FULMAR:
#ifdef BOARD_CRANEW_MCU
            psram_cranew_mcu_fc_config(PSRAM_FREQ_416M);                            //Z2
#else
            psram_phy_fc_config_craneg_a0(PSRAM_FREQ_416M, PSRAM_PCCLK_PLL1_350M);  //z2
#endif
            break;

        default:
            /*AP_4M and WB_4M spec not support freq > 400Mbps */
            if (0 == strcmp(prop_str, "AP_4M") || 0 == strcmp(prop_str, "WB_4M")) {
                psram_phy_fc_config_default(PSRAM_FREQ_350M, PSRAM_PCCLK_PLL1_350M);
            }
            else {
                /* in future chip, all use PSRAM_PCCLK_PLL1_350M */
                psram_phy_fc_config_default(PSRAM_FREQ_416M, PSRAM_PCCLK_PLL1_350M);
            }
            break;
    }
}
#endif

void psram_test(void)
{
    LOG_INFO("[PSRAM] psram test start ----\n");

    writel(0xabcd, 0x7e000000);
    LOG_INFO("[PSRAM] 0x7e000000(0xabcd) = 0x%x\n", (unsigned int)readl(0x7e000000));

    writel(0xabcd, 0x7e000010);
    LOG_INFO("[PSRAM] 0x7e000010(0xabcd) = 0x%x\n", (unsigned int)readl(0x7e000010));

    writel(0x600, 0x7e000010);
    LOG_INFO("[PSRAM] 0x7e000000(0xabcd) = 0x%x\n", (unsigned int)readl(0x7e000000));
    writel(0xea, 0x7e000004);
    LOG_INFO("[PSRAM] 0x7e000000(0xabcd) = 0x%x\n", (unsigned int)readl(0x7e000000));

    LOG_INFO("[PSRAM] 0x7e000010(0x600) = 0x%x\n", (unsigned int)readl(0x7e000010));
    LOG_INFO("[PSRAM] 0x7e000004(0xea) = 0x%x\n", (unsigned int)readl(0x7e000004));

    memset((void *)0x7e000000, 0x06, 32);
    LOG_INFO("[PSRAM] after mem set 0x7e000000(0x6060606) = 0x%x\n", (unsigned int)readl(0x7e000000));
    LOG_INFO("[PSRAM] after mem set 0x7e000010(0x6060606) = 0x%x\n", (unsigned int)readl(0x7e000010));
    LOG_INFO("[PSRAM] after mem set 0x7e000020(x) = 0x%x\n", (unsigned int)readl(0x7e000020));

    // it is necessary to test the end address of psram.
/*
    memset((void *)0x7efff000, 0x5a, 32);
    LOG_INFO("[PSRAM] after mem set 0x7efff000(0x5a5a5a5a) = 0x%x\n", (unsigned int)readl(0x7efff000));
    LOG_INFO("[PSRAM] after mem set 0x7efff010(0x5a5a5a5a) = 0x%x\n", (unsigned int)readl(0x7efff010));

    memset((void *)0x7ff00000, 0x5a, 32);
    LOG_INFO("[PSRAM] after mem set 0x7ff00000(0x5a5a5a5a) = 0x%x\n", (unsigned int)readl(0x7ff00000));
    LOG_INFO("[PSRAM] after mem set 0x7ff00010(0x5a5a5a5a) = 0x%x\n", (unsigned int)readl(0x7ff00010));
    LOG_INFO("[PSRAM] after mem set 0x7ff0001C(0x5a5a5a5a) = 0x%x\n", (unsigned int)readl(0x7ff0001C));
 */
    LOG_INFO("[PSRAM] psram test end ----\n");
}
