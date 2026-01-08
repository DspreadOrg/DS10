#ifndef __AUTH_NVCTR_H__
#define __AUTH_NVCTR_H__

#include "tbbr_img_def.h"
#include "secureboot_arom_patch.h"

#define SUPPORT_AUTH_NVCTR (AROM_VERSION == AROM_VER_2020_07_30_CRANEGM_A0 || \
                            AROM_VERSION == AROM_VER_2021_10_16_CRANEL_A0 || \
                            AROM_VERSION == AROM_VER_2022_11_11_CRANEG_CR5 || \
                            AROM_VERSION == AROM_VER_2022_11_06_CRANEL_CR5)
/*---------------------------------------------------------------------------*/
typedef union __attribute__((packed, aligned(1))) {
  struct {
    uint32_t minor : 24; /*bit[23:0], flash based nv counters */
    uint32_t major : 8; /*bit[31:24], fuse based nv counters */
  } s;
  uint32_t nv_ctr;
} nvctr_t;
/*---------------------------------------------------------------------------*/
typedef struct __attribute__((packed, aligned(1))) {
  /* Must be the first item */
  uint32_t init_value;
  nvctr_t nv_counters[MAX_NUMBER_IDS + 1];
  /* Must be last item, so that it can be written separately after the main
   * write operation has succeeded
   */
  uint32_t swap_count;
} flash_otp_nv_ctr_region_t;

int auth_get_plat_nvctr(nvctr_t *nvctr_plat);
int auth_get_cert_nvctr(void *fip_start, nvctr_t *nvctr_cert);
int auth_get_image_info_data(void *fip_start, unsigned int img_id,
                             uint64_t *addr, uint64_t *sz);
/*
 * Authenticate by Non-Volatile counter
 *
 * To protect the system against rollback, the platform includes a non-volatile
 * counter whose value can only be increased. All certificates include a counter
 * value that should not be lower than the value stored in the platform. If the
 * value is larger, the counter in the platform must be updated to the new
 * value.
 *
 * Return: 0 = success, Otherwise = error
 */
int auth_anti_rollback(nvctr_t img_nv_ctr, nvctr_t plat_nv_ctr);
#endif /* __AUTH_NVCTR_H__ */
