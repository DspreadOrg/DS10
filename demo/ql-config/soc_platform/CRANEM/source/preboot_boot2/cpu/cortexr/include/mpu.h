#ifndef _MPU_H_
#define _MPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Region number 0 has the lowest priority.
 * Region number 15 is reserved for ROM code
 */
#define MAX_PRIORITY_USER_REGION_NUM 14
#define MAX_USER_REGION_NUM 14
#define BTCM_CODE_REGION_NUM 13 /* For BTCM code */
/*#define ROM_CODE_REGION_NUM 14 / * For ROM code * / */
/*#define SQU_CODE_REGION_NUM 15 / * For SQU code * / */

/* The MPU region access control definitions */
#define MPU_REGION_ACCESS_CTRL_B      (1 << 0)              /* Bufferable */
#define MPU_REGION_ACCESS_CTRL_C      (1 << 1)              /* Cacheable */
#define MPU_REGION_ACCESS_CTRL_S      (1 << 2)              /* Shared */

/*
 * TEX: 0x1, non-cacheable
 */
#define MPU_REGION_ACCESS_CTRL_TEX(x) (((x) & 0x7) << 3)    /* Type Extensions */
/*
 * AP: 0, Permission fault
 *     1, Privileged Access only
 *     2, No user-mode write
 *     3, Full access
 */
#define MPU_REGION_ACCESS_CTRL_AP(x)  (((x) & 0x7) << 8)    /* Access Permissions */

#define MPU_REGION_ACCESS_CTRL_XN     (1 << 12)             /* Execute Never */

/* The MPU region base address should be aligned with the size and the max size is 4GB */
int mpu_region_config(unsigned num, unsigned base, unsigned size, unsigned acc_ctrl);
int mpu_config_rom(unsigned base, unsigned size, unsigned acc_ctrl);
int mpu_enable_max_priority_region(unsigned base, unsigned size, unsigned acc_ctrl);
void mpu_disable_max_priority_region(void);
int mpu_get_next_empty_region(void);
void mpu_disable_all_user_region(void);
void mpu_disable_user_used_region(void);
int set_mpu_btcm(uintptr_t base, size_t size);

#endif
