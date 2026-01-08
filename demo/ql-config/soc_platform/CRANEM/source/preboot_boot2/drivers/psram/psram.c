#include "psram.h"
#include "reg.h"
#include "cpu.h"
#include <stdio.h>
#include <string.h>

#include "log.h"
#define LOG_MODULE "Psram"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define PSRAM_REG32(x) *((volatile unsigned long *)(x))

#define APB_SPARE           (0xd4090000)
#define PLL1_SW_CTRL2       (APB_SPARE + 0x104)

#define PMU_MC_HW_SLP_TYPE  (PMU_BASE + 0x0b0)

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

static unsigned int psram_get_max_freq(void)
{
    unsigned phy_clk_level;

    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            phy_clk_level = PSRAM_FREQ_416M;
            break;
        default:
            phy_clk_level = PSRAM_FREQ_350M;
            break;
    }

    return phy_clk_level;
}

void psram_init(void)
{
    if (hw_platform_type() == HW_PLATFORM_TYPE_FPGA) {
        extern void psram_init_fpga(void);
        psram_init_fpga();
        return;
    }
    else if (hw_platform_type() == HW_PLATFORM_TYPE_ZEBU_Z1) {
        LOG_INFO("HW_PLATFORM_TYPE_ZEBU_Z1: return\n");
        return;
    }

    psram_pll1_enable();
    psram_phy_fc_config(psram_get_min_freq());

    unsigned int chip_id = hw_chip_id();
    if (CHIP_ID_CRANE == chip_id) {
      extern void psram_init_crane_a0(void);
      psram_init_crane_a0();
    } else {
      extern void psram_init_craneg_a0(void);
      psram_init_craneg_a0();
    }

    psram_phy_fc_config(psram_get_max_freq());
}

/*
 * APB_spare2_reg: PLL1 SW control Register2
 * Z1 need manually enable PLL1, Z2  PLL1 is already enabled by default.
 */
void psram_pll1_enable(void)
{
    PSRAM_REG32(PLL1_SW_CTRL2) |= 0x3F;
}

/*********************************************************************/
/* PCCLK Clk for PHY CLK Select :
 * 0x0 = PLL1 312 MHz
 * 0x1 = PLL1 350 MHz
 * 0x2 = PLL1 416 MHz
 * 0x5 = PLL1 156 MHz

 * PHY Clk Divider:
 * PHY Clk = PHY_SEL / (this field +1)

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
 */
static const struct PsramPP_t PsramPP_crane[] =
{
    /*LV0:   78M*/ { 78, 0x5, 1 },      // PLL1_156 -- 156/2 clock -- 78MHz
    /*LV1:  156M*/ { 156, 0x0, 1 },     // PLL1_312 -- 312/2 clock -- 156MHz
    /*LV2:  175M*/ { 175, 0x1, 1 },     // PLL1_350 -- 350/2 clock -- 175MHz
    /*LV3:  208M*/ { 208, 0x2, 1 },     // PLL1_416 -- 416/2 clock -- 208MHz

    // the PP below is usually used for UHS
    /*LV4:  1066M*/ { 1066, 0x4, 1 },           // PLL2_DIV2 -- 2133/2
    /*LV5:  711M*/ { 711, 0x5, 1 },             // PLL2_DIV3 -- 2133/3
    /*LV6:  533M*/ { 533, 0x6, 1 },             // PLL2_DIV4 -- 2133/4
    /*LV7:  2133M*/ { 2133, 0x7, 1 },           // PLL2_DIV1 -- 2133/1
};

/*
 * Crane, Craneg Z1/Z2 and Crane M
 * phyclk_level, bit[20:18]: min:            LV0, dclk_level=0(78M,  PLL1_156);
 *                           max in preboot: LV2, dclk_level=2(175M, PLL1_350);
 * pcclk_sel,    bit[28:26]: always                          0(      PLL1_312);
 */
static void psram_phy_fc_config_default(unsigned dclk_level, unsigned pcclk_sel)
{
    unsigned reg_val = 0;
    unsigned phyclk_fc_en = 1;
    unsigned phyclk_sel = 0;
    unsigned phyclk_div = 0;
    unsigned high_fp = 0;

    //phy clk is dividered
    LOG_INFO("PSRAM PHY frequency changed to %d\n", PsramPP_crane[dclk_level].freq);

    phyclk_sel = PsramPP_crane[dclk_level].clkSel;
    phyclk_div = PsramPP_crane[dclk_level].clkDiv;
    if (dclk_level >= 1) {
        high_fp = 1;
    }
    //if (dclk_level >= 1) {
    //    pcclk_sel = 0;                // Z1 or A0 pcclk <= pscclk signoff slow 312; Z1: pcclk <= pscclk signoff slow 307
    //}
    reg_val = PSRAM_REG32(PMU_MC_HW_SLP_TYPE);
    reg_val &= ~((0x7 << 26) | (0x7 << 18) | (0x3 << 16) | (0x3 << 4)); // don't touch bit3, HW will automatically switch b/w LP and FC.
    reg_val |= (pcclk_sel << 26) | (phyclk_sel << 18) | (phyclk_div << 16) | (phyclk_fc_en << 5) | (high_fp << 4);
    reg_val |= (1 << 24);                                               // PCCLK_AND_PHYCLK Frequency Change Request
    PSRAM_REG32(PMU_MC_HW_SLP_TYPE) = reg_val;
    while (PSRAM_REG32(PMU_MC_HW_SLP_TYPE) & (1 << 24)) {}

    //LOG_INFO("pcclk_sel=%d, phyclk_sel=%d, phyclk_div=%d\n", pcclk_sel, phyclk_sel, phyclk_div);
    //LOG_INFO("PSRAM PHY frequency change done\r\n: reg@[0x%08X]=[0x%08X].\n", (unsigned int)PMU_MC_HW_SLP_TYPE, (unsigned int)PSRAM_REG32(PMU_MC_HW_SLP_TYPE));
}

/*
 * CraneG_A0
 * phyclk_level, bit[20:18]: min:            LV1, dclk_level=1(156M, PLL1_312);
 *                           max in preboot: LV3, dclk_level=3(208M, PLL1_416);
 * pcclk_sel,    bit[28:26]: always                          1(      PLL1_350);
 */
static void psram_phy_fc_config_craneg_a0(unsigned phyclk_level, unsigned pcclk_sel)
{
    unsigned reg_val = 0;
    //unsigned pcclk_sel = 1; // psc controller clock: 350MHz // PCCLK Clk Select<p>0x0 = PLL1 312 MHz<p>0x1 = PLL1 350 MHz<p>0x2 = PLL1 416 MHz<p>0x3 = 499MHz<p>0x4 = PLL1 208 MHz<p>0x5 = PLL1 156 MHz<p>0x6 & 0x7 = reserved
    unsigned phyclk_sel = 0;
    unsigned phyclk_div = 0;
    unsigned fp_num = 1; // NON UHS: 2 FP; UHS: 4 FP

    //phy clk is dividered
    LOG_INFO("PSRAM PHY frequency changed to %d\n", PsramPP_crane[phyclk_level].freq);

    phyclk_sel = PsramPP_crane[phyclk_level].clkSel;
    phyclk_div = PsramPP_crane[phyclk_level].clkDiv;

    reg_val = PSRAM_REG32(PMU_MC_HW_SLP_TYPE);
    reg_val &= ~((0x7 << 26) | (0x7 << 18) | (0x3 << 16) | (0xf << 3)); // don't touch bit3, HW will automatically switch b/w LP and FC.
    reg_val |=  (pcclk_sel << 26) | (phyclk_sel << 18) | (phyclk_div << 16);
    reg_val |= (1 << 6) | (fp_num << 4);                                // bit6: phy & psc clk fc enable; bit[4:3]: fp_num, mc_fc_type Bit[6:3]
    reg_val |= (1 << 24);                                               // PCCLK_AND_PHYCLK Frequency Change Request
    PSRAM_REG32(PMU_MC_HW_SLP_TYPE) = reg_val;
    while (PSRAM_REG32(PMU_MC_HW_SLP_TYPE) & (1 << 24)) {}

    //LOG_INFO("pcclk_sel=%d, phyclk_sel=%d, phyclk_div=%d\n", pcclk_sel, phyclk_sel, phyclk_div);
    //LOG_INFO("PSRAM PHY frequency change done\r\n: reg@[0x%08X]=[0x%08X].\n", (unsigned int)PMU_MC_HW_SLP_TYPE, (unsigned int)PSRAM_REG32(PMU_MC_HW_SLP_TYPE));
}

void psram_phy_fc_config(unsigned dclk_level)
{
    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            psram_phy_fc_config_craneg_a0(dclk_level, PSRAM_PCCLK_PLL1_350M);
            break;
        default:
            psram_phy_fc_config_default(dclk_level, PSRAM_PCCLK_PLL1_312M);
            break;
    }
}

void psram_phy_fc_config_with_pcclk_level(unsigned dclk_level, unsigned pcclk_sel)
{
    unsigned int chip_id = hw_chip_id();

    switch (chip_id) {
        case CHIP_ID_CRANEG:
            psram_phy_fc_config_craneg_a0(dclk_level, pcclk_sel);
            break;
        default:
            psram_phy_fc_config_default(dclk_level, pcclk_sel);
            break;
    }
}

void psram_test(void)
{
    LOG_INFO("[PSRAM] psram_test --\n");
    //single write
    writel(0xabcd, 0x7e000000);
    LOG_INFO("[PSRAM] 0x7e000000  = %x\n", (unsigned int)readl(0x7e000000));

    writel(0xabcd, 0x7e000010);
    LOG_INFO("[PSRAM] 0x7e000010  = %x\n", (unsigned int)readl(0x7e000010));

    writel(0x0600, 0x7e000010);
    LOG_INFO("[PSRAM] 0x7e000000  = %x\n", (unsigned int)readl(0x7e000000));
    writel(0x00ea, 0x7e000004);
    LOG_INFO("[PSRAM] 0x7e000000  = %x\n", (unsigned int)readl(0x7e000000));

    LOG_INFO("[PSRAM] 0x7e000010  = %x\n", (unsigned int)readl(0x7e000010));
    LOG_INFO("[PSRAM] 0x7e000004  = %x\n", (unsigned int)readl(0x7e000004));


#if 0
    //mem set
    memset((void *)0x7e000000, 0x06, 32);
    LOG_INFO("%x", readl(0x7e000000));

    LOG_INFO("[PSRAM] after mem set0x7e000000  = %x", (unsigned int)readl(0x7e000000));
    LOG_INFO("[PSRAM] after mem set0x7e000010  = %x", (unsigned int)readl(0x7e000010));
    LOG_INFO("[PSRAM] after mem set0x7e000020  = %x", (unsigned int)readl(0x7e000020));
#endif

    LOG_INFO("[PSRAM] psram_test end --\n");

}
