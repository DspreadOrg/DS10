#include "mfp-drv.h"
#include "mfp.h"

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
void
spi_mfp_config(void)
{
  mfp_config((uint32_t *)mfp_cfgs_spi);
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
