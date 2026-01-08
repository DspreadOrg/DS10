#include <stdio.h>
#include <stdint.h>
#include "freq.h"
#include "reg.h"

#include "log.h"
#define LOG_MODULE "Freq"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
/* PMU AP base address */
#define PMU_BASE 0xD4282800

/* PMU AP register offset */
#define PMUA_CP_CPU_CLK_CTRL    0x10C
#define PMUA_ACLK_CTRL          0x144

/* CP CPU CLK Control register bit */
#define CP_CORE_ACLK_DIV_SHIFT      9
#define CP_CORE_MC_CLK_DIV_SHIFT    6
#define CP_CORE_PCLK_DIV_SHIFT      3
#define CP_CORE_PCLK_SEL_SHIFT      0
#define CP_CORE_CLK_FC_REQ          (1 << 12)

/* ACLK Control register bit */
#define ACLK_FC_REQ     (1 << 4)
#define ACLK_SEL        (1 << 0)

/* PLL1 SW control register 2 */
#define APB_SPARE2_REG 0xD4090104

#define CLK_SEL_MASK    0x7
#define CLK_DIV_MASK    0x7

/*---------------------------------------------------------------------------*/
void
cr5_freq_change(unsigned pclk_sel, unsigned pclk_div)
{
  unsigned val, mask;
  unsigned aclk_div = 1, mc_clk_div = 1;
  int i;

  /* Enable PPL1 307, 350, 409, 491, 614, 819 */
  writel(readl(APB_SPARE2_REG) | (0x3F), APB_SPARE2_REG);

  mask = (CLK_SEL_MASK << CP_CORE_PCLK_SEL_SHIFT) |
    (CLK_DIV_MASK << CP_CORE_MC_CLK_DIV_SHIFT) |
    (CLK_DIV_MASK << CP_CORE_PCLK_DIV_SHIFT) |
    (CLK_DIV_MASK << CP_CORE_ACLK_DIV_SHIFT);

  val = readl(PMU_BASE + PMUA_CP_CPU_CLK_CTRL);
  val &= ~mask;
  val |= (pclk_sel << CP_CORE_PCLK_SEL_SHIFT) |
    (pclk_div << CP_CORE_PCLK_DIV_SHIFT) |
    (mc_clk_div << CP_CORE_MC_CLK_DIV_SHIFT) |
    (aclk_div << CP_CORE_ACLK_DIV_SHIFT) |
    CP_CORE_CLK_FC_REQ;
  writel(val, PMU_BASE + PMUA_CP_CPU_CLK_CTRL);

  for(i = 0; i < 100000; i++) {
    val = readl(PMU_BASE + PMUA_CP_CPU_CLK_CTRL);
    if((val & CP_CORE_CLK_FC_REQ) == 0) {
      break;
    }
  }

  if(val & CP_CORE_CLK_FC_REQ) {
    LOG_ERR("CR5 freq change failed\n");
  }
}
/*---------------------------------------------------------------------------*/
void
axi_freq_change(unsigned int aclk_sel)
{
  unsigned val;
  int i;

  val = readl(PMU_BASE + PMUA_ACLK_CTRL);
  val &= ~ACLK_SEL;
  val |= (aclk_sel & ACLK_SEL) | ACLK_FC_REQ;
  writel(val, PMU_BASE + PMUA_ACLK_CTRL);

  for(i = 0; i < 100000; i++) {
    val = readl(PMU_BASE + PMUA_ACLK_CTRL);
    if((val & ACLK_FC_REQ) == 0) {
      break;
    }
  }

  if(val & ACLK_FC_REQ) {
    LOG_ERR("AXI freq change failed\n");
  }
}
/*---------------------------------------------------------------------------*/
/* pclk: CR5 clk; aclk AXI clk */
int
get_curr_freq(unsigned *ppclk, unsigned *paclk)
{
  unsigned cr5_val, axi_val, clk_sel, pclk_div;
  unsigned cr5_clk[5] = { 416, 312, 499, 624, 832 };
  unsigned axi_clk[2] = { 156, 208 };

  cr5_val = readl(PMU_BASE + PMUA_CP_CPU_CLK_CTRL);

  clk_sel = cr5_val & CLK_SEL_MASK;
  pclk_div = (cr5_val >> CP_CORE_PCLK_DIV_SHIFT) & CLK_DIV_MASK;

  if(clk_sel > 4) {
    LOG_ERR("get_curr_freq failed, Invalid clk_sel, 0x%x = 0x%x", PMU_BASE + PMUA_CP_CPU_CLK_CTRL, cr5_val);
    return -1;
  } else if(clk_sel == 4) {
    *ppclk = cr5_clk[clk_sel];
  } else {
    *ppclk = cr5_clk[clk_sel] / (1 + pclk_div);
  }

  axi_val = readl(PMU_BASE + PMUA_ACLK_CTRL);
  *paclk = axi_clk[axi_val & ACLK_SEL];

  return 0;
}
/*---------------------------------------------------------------------------*/
void
cr5_axi_set_default_freq(void)
{
  unsigned pclk_old, aclk_old, pclk, aclk;

  if(get_curr_freq(&pclk_old, &aclk_old) < 0) {
    goto error;
  }

  cr5_freq_change(CR5_PCLK_SEL_416, CR5_PCLK_DIV_0);    /* CR5 416MHz */
  axi_freq_change(AXI_FREQ_156M);                       /* AXI 156MHz */

  if(get_curr_freq(&pclk, &aclk) < 0) {
    goto error;
  }

  LOG_INFO("Freq change done: CR5 %uMHz -> %uMHz, AXI %uMHz -> %uMHz\n",
           pclk_old, pclk, aclk_old, aclk);
  return;
error:
  LOG_ERR("cr5_axi_set_default_freq failed\n");
}
/*---------------------------------------------------------------------------*/
void
cr5_axi_set_max_freq(void)
{
  unsigned pclk_old, aclk_old, pclk, aclk;

  if(get_curr_freq(&pclk_old, &aclk_old) < 0) {
    goto error;
  }

  cr5_freq_change(CR5_PCLK_SEL_624, CR5_PCLK_DIV_0);    /* CR5 624MHz */
  axi_freq_change(AXI_FREQ_208M);                       /* AXI 208MHz */

  if(get_curr_freq(&pclk, &aclk) < 0) {
    goto error;
  }

  LOG_INFO("Freq change done: CR5 %uMHz -> %uMHz, AXI %uMHz -> %uMHz\n",
           pclk_old, pclk, aclk_old, aclk);
  return;
error:
  LOG_ERR("cr5_axi_set_max_freq failed\n");
}
/*---------------------------------------------------------------------------*/
