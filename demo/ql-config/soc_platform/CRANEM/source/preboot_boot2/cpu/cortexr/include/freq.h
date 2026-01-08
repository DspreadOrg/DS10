#ifndef __FREQ_H__
#define __FREQ_H__

/* pclk_sel */
#define CR5_PCLK_SEL_416    0
#define CR5_PCLK_SEL_312    1
#define CR5_PCLK_SEL_499    2
#define CR5_PCLK_SEL_624    3
#define CR5_PCLK_SEL_832    4

/* pclk_div */
#define CR5_PCLK_DIV_0      0
#define CR5_PCLK_DIV_1      1
#define CR5_PCLK_DIV_2      2
#define CR5_PCLK_DIV_3      3

void cr5_freq_change(unsigned pclk_sel, unsigned pclk_div);

/* aclk_sel */
#define AXI_FREQ_156M       0
#define AXI_FREQ_208M       1

void axi_freq_change(unsigned int aclk_sel);

/* pclk: CR5 clk; aclk AXI clk */
int get_curr_freq(unsigned *ppclk, unsigned *paclk);

void cr5_axi_set_default_freq(void);
void cr5_axi_set_max_freq(void);

#endif
