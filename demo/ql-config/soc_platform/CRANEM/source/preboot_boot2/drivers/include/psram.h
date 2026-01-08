#ifndef PERIPH_PSRAM_H
#define PERIPH_PSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

struct PsramPP_t {
  unsigned freq;
  unsigned clkSel;      /*clock selection ( pc clk select: bit28~26, phy clk select: bit20~18,) */
  unsigned clkDiv;      /*phy clk divider (bit17~16) */
};

/* phy clk = phy_sel/phy_clk_divider+1, eg: phy clk = phy_sel/2 = 156/2=78M*/
#define PSRAM_FREQ_156M     0   /*LV0:   78M*/
#define PSRAM_FREQ_312M     1   /*LV1:  156M*/
#define PSRAM_FREQ_350M     2   /*LV2:  175M*/
#define PSRAM_FREQ_416M     3   /*LV3:  208M*/
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

void psram_init(void);
void psram_pll1_enable(void);
void psram_test(void);
void psram_phy_fc_config(unsigned dclk_level);
void psram_phy_fc_config_with_pcclk_level(unsigned dclk_level, unsigned pcclk_sel);

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_PSRAM_H */
/** @} */
