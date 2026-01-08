#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "contiki.h"
#include "cpu.h"
#include "secureboot_arom_patch.h"
#include "reg.h"
#include "fip.h"
#include "tbbr_img_def.h"
#include "nvctr_flash.h"
#include "auth_nvctr.h"
#include "ptable.h"

/* Log configuration */
#include "log.h"
#define LOG_MODULE                    "NVCtr"
#define LOG_LEVEL                     LOG_LEVEL_MAIN

#define SYS_BOOT_CNTRL                (CIU_BASE + 0x20)
#define SECURE_BOOT_ENABLE            (1 << 20)
#define NEED_GET_AROM_FUNC            (AROM_VERSION == AROM_VER_2021_10_16_CRANEL_A0 || \
                                       AROM_VERSION == AROM_VER_2022_11_11_CRANEG_CR5 || \
                                       AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5)
/*---------------------------------------------------------------------------*/
static const auth_img_desc_t *arom_cot_desc_ptr;
static bool has_nvctr_cert;
/*---------------------------------------------------------------------------*/
#if NEED_GET_AROM_FUNC
/* arom interfaces */
typedef int (*img_parser_check_integrity_t)(img_type_t img_type,
                                            void *img_ptr, unsigned int img_len);
static img_parser_check_integrity_t img_parser_check_integrity;
int
arom_img_parser_check_integrity(img_type_t img_type,
                                void *img_ptr, unsigned int img_len)
{
  return img_parser_check_integrity(img_type, img_ptr, img_len);
}
/*---------------------------------------------------------------------------*/
typedef int (*img_parser_get_auth_param_t)(img_type_t img_type,
                                           const auth_param_type_desc_t *type_desc,
                                           void *img_ptr, unsigned int img_len,
                                           void **param_ptr, unsigned int *param_len);
/*---------------------------------------------------------------------------*/
static img_parser_get_auth_param_t img_parser_get_auth_param;
int
arom_img_parser_get_auth_param(img_type_t img_type,
                               const auth_param_type_desc_t *type_desc,
                               void *img_ptr, unsigned int img_len,
                               void **param_ptr, unsigned int *param_len)
{
  return img_parser_get_auth_param(img_type, type_desc, \
                                   img_ptr, img_len, param_ptr, param_len);
}
#endif
/*---------------------------------------------------------------------------*/
static void
arom_refs_init(void)
{
  static bool inited = false;

  if(inited) {
    return;
  }

#if AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0
  arom_cot_desc_ptr = cot_desc_ptr;
  /*
   * arom_img_parser_get_auth_param() is defined in secureboot_arom_patch.c,
   * which is included in AROM_VERSION 2020.07.30.
   */
#else
  switch(hw_chip_id()) {
  case CHIP_ID_CRANEL:
    arom_cot_desc_ptr = (auth_img_desc_t *)0xD1001B00;
    img_parser_check_integrity = (img_parser_check_integrity_t)(0xD1F067C0 + 1);
    img_parser_get_auth_param = (img_parser_get_auth_param_t)(0xD1F067E0 + 1);
    break;
  case CHIP_ID_CRANELS:
    arom_cot_desc_ptr = (auth_img_desc_t *)0xD1002C40;
    img_parser_check_integrity = (img_parser_check_integrity_t)(0xD1F07240 + 1);
    img_parser_get_auth_param = (img_parser_get_auth_param_t)(0xD1F07260 + 1);
    break;
  case CHIP_ID_CRANELR:
    arom_cot_desc_ptr = (auth_img_desc_t *)0xD1002C34;
    img_parser_check_integrity = (img_parser_check_integrity_t)(0xD1F0720C + 1);
    img_parser_get_auth_param = (img_parser_get_auth_param_t)(0xD1F0722C + 1);
    break;
  case CHIP_ID_CRANEGT:
    if(hw_rev_id() == REV_ID_CRANEGT_A0) {
      arom_cot_desc_ptr = (auth_img_desc_t *)0xD1008E30;
      img_parser_check_integrity = (img_parser_check_integrity_t)(0xD1F08A44 + 1);
      img_parser_get_auth_param = (img_parser_get_auth_param_t)(0xD1F08A64 + 1);
    } else { /* For CRANEGT A1 */
      arom_cot_desc_ptr = (auth_img_desc_t *)0xD1008E50;
      img_parser_check_integrity = (img_parser_check_integrity_t)(0xD1F08B28 + 1);
      img_parser_get_auth_param = (img_parser_get_auth_param_t)(0xD1F08B48 + 1);
    }
    break;
  default:
    LOG_ERR("%s, invalid chip id: 0x%x\n", __func__, hw_chip_id());
    break;
  }
#endif

  inited = true;
}
/*---------------------------------------------------------------------------*/
static bool
geu_trust_boot_enabled(void)
{
  unsigned int sys_boot_ctrl = readl(SYS_BOOT_CNTRL);
  unsigned int sbe = sys_boot_ctrl & SECURE_BOOT_ENABLE;

  return sbe != 0;
}
/*---------------------------------------------------------------------------*/
static const auth_method_param_nv_ctr_t *
get_nv_ctr_param(void)
{
  int rc = -1;
  const auth_img_desc_t *img_desc = NULL;
  const auth_method_desc_t *auth_method = NULL;

  img_desc = &arom_cot_desc_ptr[TRUSTED_BOOT_FW_CERT_ID];
  for(unsigned i = 0; i < AUTH_METHOD_NUM; i++) {
    auth_method = &img_desc->img_auth_methods[i];
    if(auth_method->type == AUTH_METHOD_NV_CTR) {
      rc = 0;
      break;
    }
  }

  if(rc != 0) {
    return NULL;
  }

  return &auth_method->param.nv_ctr;
}
/*---------------------------------------------------------------------------*/
static bool
nvctr_major_enabled(void)
{
  switch(hw_chip_id()) {
  default:
    return false;
  }
}
/*---------------------------------------------------------------------------*/
static bool
has_nv_counter_partition(void)
{
  struct ptentry *pt_nv = ptable_find_entry(NV_CTR_PART_NAME);
  if(pt_nv) {
    return true;
  }

  return false;
}
/*---------------------------------------------------------------------------*/
int
auth_get_image_info_data(void *fip_start, unsigned int img_id,
                         uint64_t *addr, uint64_t *sz)
{
  return fip_get_image(fip_start, img_id, addr, sz);
}
/*---------------------------------------------------------------------------*/
int
auth_get_plat_nvctr(nvctr_t *nvctr_plat)
{
  int rc;
  const auth_method_param_nv_ctr_t *param;

  if(!geu_trust_boot_enabled()) {
    return 0;
  }

  /* Initialize the nv counter from the platform */
  rc = plat_init_nv_ctr();
  if(rc < 0) {
    return rc;
  } else if(rc == 0) {/* nv counter partition exist. */
    if(has_nvctr_cert == false) {
      return_if_error(-1);
    }
  }

  param = get_nv_ctr_param();
  /* Get NV counter from the platform */
  rc = plat_get_nv_ctr(param->plat_nv_ctr->cookie, nvctr_plat);
  return_if_error(rc);
  LOG_INFO("Platform nv counter: 0x%lx\n", nvctr_plat->nv_ctr);

  return 0;
}
/*---------------------------------------------------------------------------*/
int
auth_get_cert_nvctr(void *fip_start, nvctr_t *nvctr_cert)
{
  const auth_img_desc_t *img_desc = NULL;
  const auth_method_param_nv_ctr_t *param;
  void *data_ptr = NULL;
  unsigned int data_len, len;
  uint32_t cert_nv_ctr;
  uint64_t addr, size;
  char *p;
  void *img;
  int rc;

  has_nvctr_cert = true;
  if(!geu_trust_boot_enabled()) {
    LOG_INFO("Non-trusted mode, skip authorize NV counters.\n");
    return 0;
  } else {
    LOG_INFO("Trusted mode, authorize NV counters.\n");
  }

  arom_refs_init();

  if(!is_valid_fip(fip_start)) {
    LOG_INFO("No certificate nv counter\n");
    has_nvctr_cert = false;
    return 0;
  }

  rc = auth_get_image_info_data(fip_start, TRUSTED_BOOT_FW_CERT_ID,
                                &addr, &size);
  return_if_error(rc);

  img = (uintptr_t)addr;
  img_desc = &arom_cot_desc_ptr[TRUSTED_BOOT_FW_CERT_ID];

  /* Ask the parser to check the image integrity */
  rc = arom_img_parser_check_integrity(img_desc->img_type, img, size);
  return_if_error(rc);

  param = get_nv_ctr_param();
  /* Get the counter value from current image. The AM expects the IPM
   * to return the counter value as a DER encoded integer */
  rc = arom_img_parser_get_auth_param(img_desc->img_type, param->cert_nv_ctr,
                                      img, size, &data_ptr, &data_len);
  return_if_error(rc);

  /* Parse the DER encoded integer */
  assert(data_ptr);
  p = (char *)data_ptr;

  if(*p != ASN1_INTEGER) {
    /* Invalid ASN.1 integer */
    return_if_error(-1);
  }
  p++;

  /* NV-counters are unsigned integers up to 32-bit */
  len = (unsigned int)(*p & 0x7f);
  if((*p & 0x80) || (len > 4)) {
    return_if_error(-1);
  }
  p++;

  /* Check the number is not negative */
  if(*p & 0x80) {
    return_if_error(-1);
  }

  /* Convert to unsigned int. This code is for a little-endian CPU */
  cert_nv_ctr = 0;
  for(unsigned i = 0; i < len; i++) {
    cert_nv_ctr = (cert_nv_ctr << 8) | *p++;
  }

  LOG_INFO("Certification nv counter: 0x%lx\n", cert_nv_ctr);
  nvctr_cert->nv_ctr = cert_nv_ctr;

  return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * Authenticate by Non-Volatile counter
 *
 * To protect the system against rollback, the platform includes a non-volatile
 * counter whose value can only be increased. All certificates include a counter
 * value that should not be lower than the value stored in the platform. If the
 * value is larger, the counter in the platform must be updated to the new
 * value.
 *
 * nvctr_cert: nv counters from certification
 * nvctr_plat: nv counters from platform
 *
 * Return: 0 = success, Otherwise = error
 */
int
auth_anti_rollback(nvctr_t nvctr_cert, nvctr_t nvctr_plat)
{
  int rc;
  const auth_method_param_nv_ctr_t *param;

  if(!geu_trust_boot_enabled()) {
    return 0;
  }

  if(has_nvctr_cert == false) {
    if(has_nv_counter_partition()) {
      return_if_error(-1);
    } else {
      return 0;
    }
  }

  param = get_nv_ctr_param();

  if(nvctr_major_enabled()) {
    if(nvctr_cert.s.major < nvctr_plat.s.major) {
      /* Invalid NV-counter */
      LOG_ERR("Invalid NV counters!\n");
      return -1;
    } else if(nvctr_cert.s.major > nvctr_plat.s.major) {
      rc = plat_set_nv_ctr(param->plat_nv_ctr->cookie, nvctr_cert);
      return_if_error(rc);
    }
  }

  if(nvctr_cert.s.minor < nvctr_plat.s.minor) {
    /* Invalid NV-counter */
    LOG_ERR("Invalid NV counters!\n");
    return -1;
  } else if(nvctr_cert.s.minor >= nvctr_plat.s.minor) {
    rc = plat_set_nv_ctr(param->plat_nv_ctr->cookie, nvctr_cert);
    return_if_error(rc);
  }

  return 0;
}
