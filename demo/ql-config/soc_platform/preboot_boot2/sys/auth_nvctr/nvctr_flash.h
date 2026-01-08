#ifndef _AUTH_NVCTR_FLASH_H_
#define _AUTH_NVCTR_FLASH_H_

#include "auth_nvctr.h"

/*---------------------------------------------------------------------------*/
#define NO_NV_COUNTER_PARTITION       1
#define OTP_NV_COUNTERS_INITIALIZED   0xC0DE8112U
#define INVALID_FLASH_ADDRESS         UINT32_MAX
#define CHECK_NV_CTR_INDEX            BL2_IMAGE_ID
#define NV_CTR_PART_NAME              "nv_counter"
/*---------------------------------------------------------------------------*/
int plat_init_nv_ctr(void);
int plat_get_nv_ctr(void *cookie, nvctr_t *nv_ctr);
int plat_set_nv_ctr(void *cookie, nvctr_t nv_ctr);

#endif
