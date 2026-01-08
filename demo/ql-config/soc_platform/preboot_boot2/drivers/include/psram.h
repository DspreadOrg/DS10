#ifndef PERIPH_PSRAM_H
#define PERIPH_PSRAM_H
#include "syscall-arom.h"

#ifdef __cplusplus
extern "C" {
#endif

/* craneg not support clk mode, but bit8 reserved */
typedef enum {
    CLK_MODE_1X = 0,        /* 1x mode */
    CLK_MODE_2X,            /* 2x mode */
    CLK_MODE_MAX
} clk_mode_t;

/*
 * PHYCLK clk select , bit20~18
 * craneg:                  phy clk = phy_sel/(phy_clk_divider+1),           eg: phy clk = phy_sel/2 = 156/2=78M
 * craneLR A0/cranew cr5:   phy clk = phy_sel/(phy_clk_divider+1)/clk_mode.  clk_mode=1(1xmode), clk_mode=2(2xmode),
 * psram two edge samples, so Mbps = MHz *2;
 * If mcu is 208MHz, means it is 416Mbps(PSRAM_FREQ_416M).
 * PSRAM_FREQ_156Mbps: phy clk 78MHz
 */
struct PsramPP_t {
    unsigned freq;
    unsigned clkSel;    /*clock selection ( pc clk select: bit28~26, phy clk select: bit20~18,) */
    unsigned clkDiv;    /*phy clk divider (bit17~16) */
    clk_mode_t clkMode; /*0: 1x mode; 1: 2xmode, add after craneLR, include cranew cr5 */
};

#define PSRAM_FREQ_156M     0   /*LV0:   78M 156Mps*/
#define PSRAM_FREQ_312M     1   /*LV1:  156M 312Mps*/
#define PSRAM_FREQ_350M     2   /*LV2:  175M 350Mbps, craneLR: 356Mbps */
#define PSRAM_FREQ_416M     3   /*LV3:  208M 416Mps*/
#define PSRAM_FREQ_500M     4   /*LV4:  250M 500Mps*/

#if 0
//test lower freq can use it
#define PSRAM_FREQ_LV4      4   /*LV4:  */
#define PSRAM_FREQ_LV5      5   /*LV5:  */
#endif
/*
 * preboot will not use psram LV4 PSRAM_FREQ_533M, cp use LV4.
 * phy clk 533M ~2000M need enable PLL2, see Asic code: psram_fc_init -> phy_pll2_fc
 */
/*PCCLK clk select , bit28~26 */
#define PSRAM_PCCLK_PLL1_312M     0
#define PSRAM_PCCLK_PLL1_350M     1
#define PSRAM_PCCLK_PLL1_416M     2
#define PSRAM_PCCLK_PLL1_208M     4
#define PSRAM_PCCLK_PLL1_156M     5

#define PSRAM_CACHELINE_64B       0
#define PSRAM_CACHELINE_128B      1
#define PSRAM_CACHELINE_256B      2
#define PSRAM_CACHELINE_512B      3

void psram_init(void);
void psram_pll1_enable(void);
void psram_test(void);
void psram_phy_fc_config(unsigned phyclk_level);

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_PSRAM_H */
/** @} */
