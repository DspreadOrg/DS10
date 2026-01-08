#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freq.h"
#include "reg.h"
#include "syscall-arom.h"
#include "cpu.h"

#include "log.h"
#define LOG_MODULE "Freq"
#define LOG_LEVEL LOG_LEVEL_MAIN

/*---------------------------------------------------------------------------*/
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
#define ACLK_SEL_LEGACY (1 << 0)
#define ACLK_SEL        (3 << 0)

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
static bool
cr5_clk_legacy(void)
{
  unsigned int chip_id = hw_chip_id();

  return chip_id == CHIP_ID_CRANE ||
         chip_id == CHIP_ID_CRANEG ||
         chip_id == CHIP_ID_CRANEM ||
         chip_id == CHIP_ID_CRANEL;
}
/*---------------------------------------------------------------------------*/
static bool
axi_clk_legacy(void)
{
  unsigned int chip_id = hw_chip_id();

  return chip_id == CHIP_ID_CRANE ||
         chip_id == CHIP_ID_CRANEG ||
         chip_id == CHIP_ID_CRANEM ||
         chip_id == CHIP_ID_FULMAR ||
         chip_id == CHIP_ID_CRANEL;
}
/*---------------------------------------------------------------------------*/
/* pclk: CR5 clk; aclk AXI clk */
static const unsigned _cr5_clk[5] = { 409, 307, 491, 614, 819 };
static const unsigned _axi_clk[4] = { 154, 205, 189, 223 };
static const unsigned _cr5_clk_legacy[5] = { 416, 312, 499, 624, 832 };
static const unsigned _axi_clk_legacy[2] = { 156, 208 };
/* Only craneGT and craneLRH axi clock config are different from craneLS/LR/LG/LB etc. */
static const unsigned _axi_clk_cranelrh_gt[4] = { 223, 189, 154, 205 };
/* Lapwing axi clock config. */
static const unsigned _axi_clk_lapwing[4] = { 156, 208, 192, 312 };
/*---------------------------------------------------------------------------*/
int
get_curr_freq(unsigned *ppclk, unsigned *paclk)
{
  unsigned cr5_val, axi_val, clk_sel, pclk_div;
  const unsigned *cr5_clk, *axi_clk;

  cr5_clk = cr5_clk_legacy() ? _cr5_clk_legacy : _cr5_clk;

  cr5_val = readl(PMU_BASE + PMUA_CP_CPU_CLK_CTRL);
  clk_sel = cr5_val & CLK_SEL_MASK;
  pclk_div = (cr5_val >> CP_CORE_PCLK_DIV_SHIFT) & CLK_DIV_MASK;

  if(clk_sel > 4) {
    LOG_ERR("get_curr_freq failed, Invalid clk_sel, 0x%x = 0x%x \n",
            PMU_BASE + PMUA_CP_CPU_CLK_CTRL, cr5_val);
    return -1;
  } else if(clk_sel == 4) {
    *ppclk = cr5_clk[clk_sel];
  } else {
    *ppclk = cr5_clk[clk_sel] / (1 + pclk_div);
  }

  axi_val = readl(PMU_BASE + PMUA_ACLK_CTRL);
  unsigned int chipid = hw_chip_id();
  if(chipid == CHIP_ID_CRANELRH || chipid == CHIP_ID_CRANEGT) {
    axi_clk = _axi_clk_cranelrh_gt;
    *paclk = axi_clk[axi_val & ACLK_SEL];
  } else if(chipid == CHIP_ID_LAPWING) {
    axi_clk = _axi_clk_lapwing;
    *paclk = axi_clk[axi_val & ACLK_SEL];
  } else if(axi_clk_legacy()) {
    axi_clk = _axi_clk_legacy;
    *paclk = axi_clk[axi_val & ACLK_SEL_LEGACY];
  } else {
    axi_clk = _axi_clk;
    *paclk = axi_clk[axi_val & ACLK_SEL];
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
#define CR5_PCLK_SEL_DEFAULT   0
#define AXI_FREQ_DEFAULT       0
/*---------------------------------------------------------------------------*/
void
cr5_axi_set_default_freq(void)
{
  unsigned pclk_old, aclk_old, pclk, aclk;

  if(get_curr_freq(&pclk_old, &aclk_old) < 0) {
    goto error;
  }

  /*
   * axi_clk_legacy(): CraneG/M/L
   */
  unsigned int chip_id = hw_chip_id();
  if(axi_clk_legacy() || chip_id == CHIP_ID_CRANELRH) {
    /*
     * keep max freq with 20240223 version, and don't restore default value.
     *
     * CraneG/M/L: 208MHz
     * CraneLRH: 223MHz
     */
  } else if(chip_id == CHIP_ID_CRANELR || chip_id == CHIP_ID_CRANELS) {
    /*
     * keep max freq with 20240223 version
     *
     * CraneLR/LS: 205MHz
     */
    axi_freq_change(AXI_FREQ_208M);
  } else if(chip_id == CHIP_ID_CRANELG) {
    /*
     * keep max freq with 20240223 version
     *
     * CraneLG 154MHz
     */
    axi_freq_change(AXI_FREQ_156M);
  } else {
    cr5_freq_change(CR5_PCLK_SEL_DEFAULT, CR5_PCLK_DIV_0);
    axi_freq_change(AXI_FREQ_DEFAULT);
  }

  if(get_curr_freq(&pclk, &aclk) < 0) {
    goto error;
  }

  LOG_INFO("Freq change restore done: CR5 %uMHz -> %uMHz, AXI %uMHz -> %uMHz\n",
           pclk_old, pclk, aclk_old, aclk);
  return;
error:
  LOG_ERR("%s failed\n", __func__);
}
/*---------------------------------------------------------------------------*/
void
cr5_axi_set_max_freq(void)
{
  unsigned pclk_old, aclk_old, pclk, aclk;

  if(get_curr_freq(&pclk_old, &aclk_old) < 0) {
    goto error;
  }

  cr5_freq_change(CR5_PCLK_SEL_624, CR5_PCLK_DIV_0);    /* CR5 624MHz or 614MHZ for newer chip */

  unsigned freq_max;
  unsigned int chipid = hw_chip_id();
  if(chipid == CHIP_ID_CRANELRH || chipid == CHIP_ID_CRANEGT) {
    freq_max = AXI_FREQ_223M_LRH_GT;
  } else if((chipid == CHIP_ID_LAPWING)) {
    /* enable PLL1 DIV2, DIV23, DIV13, DIV11 */
    writel(readl(APB_SPARE12_REG) | (1 << 16 | 1 << 17 | 1 << 18 | 1 << 19), APB_SPARE12_REG);
    freq_max = AXI_FREQ_312M;
  } else if(axi_clk_legacy()) {
    freq_max = AXI_FREQ_208M;
  } else {
    /* enable PLL1 DIV11, DIV13, DIV23 */
    writel(readl(APB_SPARE12_REG) | 0x7, APB_SPARE12_REG);
    freq_max = AXI_FREQ_223M;
  }

  axi_freq_change(freq_max);

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
