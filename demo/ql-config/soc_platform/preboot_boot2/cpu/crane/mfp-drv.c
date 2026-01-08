#include "mfp-drv.h"
#include "mfp.h"
#include "syscall-arom.h"
#include "cpu.h"

/* Pin register definitions */
/* --------------------------------------------------------------- */
#define QSPI_DAT3_MPFI  (0x2C4)
#define QSPI_DAT2_MPFI  (0x2C8)
#define QSPI_DAT1_MPFI  (0x2CC)
#define QSPI_DAT0_MPFI  (0x2D0)
#define QSPI_CLK_MPFI   (0x2D4)
#define QSPI_CS1_MPFI   (0x2D8)
#define QSPI_CS2_MPFI   (0x2DC)
#define QSPI_DQM_MPFI   (0x2E0)
/* --------------------------------------------------------------- */
#define AP_UART1_CRANEGML_RXD        0x150
#define AP_UART1_CRANELR_RXD         0x1BC
#define AP_UART1_CRANELS_RXD         0x148
/* --------------------------------------------------------------- */
#if AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0
/*
 * eg: MX25U12835F, DAT3 has other function(WP/Hold pin) when spi mode. mx need always pull up dat3 when quad read.
 * bootrom use standard spi mode, so use Dat0 and Dat1.
 * bootrom need pull up Dat2 and Dat3 to disable other function of the pins.
 * bootloader will read device id to judge whether the flash is common, can recover Dat2 and Dat3 to pull down mode(hardware default).
 */
static const uint32_t mfp_cfgs_spi[] = {
  MFP_REG(QSPI_DAT3_MPFI) | MFP_DRIVE_MEDIUM | MFP_AF0 | MFP_PULL_HIGH,
  MFP_REG(QSPI_DAT2_MPFI) | MFP_DRIVE_MEDIUM | MFP_AF0 | MFP_PULL_HIGH,
  MFP_EOC
};
#endif
/* --------------------------------------------------------------- */
/* recover default mode(hardware default is pull down mode) for common flashes */
static const uint32_t mfp_cfgs_quad_spi[] = {
  MFP_REG(QSPI_DAT3_MPFI) | MFP_DRIVE_MEDIUM | MFP_AF0,
  MFP_REG(QSPI_DAT2_MPFI) | MFP_DRIVE_MEDIUM | MFP_AF0,
  MFP_EOC
};
/* --------------------------------------------------------------- */
static const uint32_t mfp_cfgs_mx[] = {
  MFP_REG(QSPI_DAT3_MPFI) | MFP_DRIVE_MEDIUM | MFP_AF0 | MFP_PULL_HIGH,
  MFP_EOC
};
/* --------------------------------------------------------------- */
/* if AROM_NEXT_DELIVER, mfp_cfgs_spi can be removed.
 * pull up Dat3 and Dat2 for read id always successfully for speacial flash. eg mx
 */
void
spi_mfp_config(void)
{
#if AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0
  mfp_config((uint32_t *)mfp_cfgs_spi);
#endif
}
/* --------------------------------------------------------------- */
void
qspi_mfp_config(void)
{
  mfp_config((uint32_t *)mfp_cfgs_quad_spi);
}
/* --------------------------------------------------------------- */
void
qspi_mfp_config_mx(void)
{
  mfp_config((uint32_t *)mfp_cfgs_mx);
}
/* --------------------------------------------------------------- */
static const uint32_t uart1_mfp_cranelr_cfgs[] = {
  MFP_REG(AP_UART1_CRANELR_RXD) | MFP_DRIVE_MEDIUM | MFP_PULL_NONE | MFP_AF0,
  MFP_EOC
};
/* --------------------------------------------------------------- */
static const uint32_t uart1_mfp_cranels_cfgs[] = {
  MFP_REG(AP_UART1_CRANELS_RXD) | MFP_DRIVE_MEDIUM | MFP_PULL_NONE | MFP_AF0,
  MFP_EOC
};
/* --------------------------------------------------------------- */
static const uint32_t uart1_mfp_cranegml_cfgs[] = {
  MFP_REG(AP_UART1_CRANEGML_RXD) | MFP_DRIVE_MEDIUM | MFP_PULL_NONE | MFP_AF0,
  MFP_EOC
};
/* --------------------------------------------------------------- */
int
uart_rx_gpio_config(void)
{
  switch(hw_chip_id()) {
  case CHIP_ID_CRANELR:
    mfp_config((uint32_t *)uart1_mfp_cranelr_cfgs);
    return 1;
  case CHIP_ID_CRANELS:
    mfp_config((uint32_t *)uart1_mfp_cranels_cfgs);
    return 1;
  case CHIP_ID_CRANEG:
  case CHIP_ID_CRANEM:
  case CHIP_ID_CRANEL:
    mfp_config((uint32_t *)uart1_mfp_cranegml_cfgs);
    return 1;
  }
  return 0;
}
/* --------------------------------------------------------------- */
