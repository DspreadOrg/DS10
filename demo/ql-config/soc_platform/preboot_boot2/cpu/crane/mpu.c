#include <stdio.h>
#include "mpu.h"
#include "system.h"

/*---------------------------------------------------------------------------*/
#define MPU_REGION_SIZE_MASK          (0x1f << 1)           /* Region size mask */
#define MPU_REGION_ENABLE             (1 << 0)              /* Region Enable */
/*---------------------------------------------------------------------------*/
static inline unsigned int
_mpu_get_region_num(void)
{
  unsigned int val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c2, 0" : "=r" (val) : : "memory", "cc");

  return val;
}
/*---------------------------------------------------------------------------*/
static inline void
_mpu_set_region_num(unsigned int val)
{
  __asm__ __volatile__ ("mcr p15, 0, %0, c6, c2, 0" : : "r" (val) : "memory", "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
static inline unsigned int
_mpu_get_region_base_addr(void)
{
  unsigned int val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 0" : "=r" (val) : : "memory", "cc");

  return val;
}
/*---------------------------------------------------------------------------*/
static inline void
_mpu_set_region_base_addr(unsigned int val)
{
  __asm__ __volatile__ ("mcr p15, 0, %0, c6, c1, 0" : : "r" (val) : "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
static inline unsigned int
_mpu_get_region_size(void)
{
  unsigned int val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 2" : "=r" (val) : : "memory", "cc");

  return (val & MPU_REGION_SIZE_MASK) >> 1;
}
/*---------------------------------------------------------------------------*/
static inline void
_mpu_set_region_size(unsigned int size)
{
  unsigned val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 2" : "=r" (val) : : "memory", "cc");
  val = (val & ~MPU_REGION_SIZE_MASK) | (size << 1);
  __asm__ __volatile__ ("mcr p15, 0, %0, c6, c1, 2" : : "r" (val) : "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
static inline unsigned int
_mpu_get_region_size_enable(void)
{
  unsigned int val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 2" : "=r" (val) : : "memory", "cc");

  return val;
}
/*---------------------------------------------------------------------------*/
static inline unsigned int
_mpu_get_region_access_ctrl(void)
{
  unsigned int acc_ctrl;

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 4" : "=r" (acc_ctrl) : : "memory", "cc");

  return acc_ctrl;
}
/*---------------------------------------------------------------------------*/
static inline void
_mpu_set_region_access_ctrl(unsigned int acc_ctrl)
{
  __asm__ __volatile__ ("mcr p15, 0, %0, c6, c1, 4" : : "r" (acc_ctrl) : "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
static inline void
_mpu_enable_region(unsigned int num, unsigned int enable)
{
  unsigned val;

  _mpu_set_region_num(num);

  __asm__ __volatile__ ("mrc p15, 0, %0, c6, c1, 2" : "=r" (val) : : "memory", "cc");
  if(enable) {
    val |= MPU_REGION_ENABLE;
  } else {
    val &= ~MPU_REGION_ENABLE;
  }
  __asm__ __volatile__ ("mcr p15, 0, %0, c6, c1, 2" : : "r" (val) : "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
static unsigned next_empty_region_num;
/*---------------------------------------------------------------------------*/
/* Base address should be aligned with the size,  the max size is 4GB */
static unsigned int
convert_region_size(unsigned long long size, unsigned int base)
{
  unsigned val;

  if(size < 32 || (size & (size - 1))) {
    printf("Size should be power of two between 32 Bytes and 4GB\n");
    goto error;
  }

  if(base & (size - 1)) {
    printf("Base must be aligned to the size\n");
    goto error;
  }

  val = 0;
  while(size) {
    val++;
    size >>= 1;
  }
  val -= 2;   /* 32 bytes-> 0x4, 4GB -> 0x1F */

  return val;
error:
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
_mpu_region_config(unsigned region_num, unsigned base, unsigned size, unsigned acc_ctrl)
{
  unsigned int region_size;

  region_size = convert_region_size(size, base);
  if(region_size == 0) {
    printf("region size is 0\n");
    goto error;
  }

  _mpu_set_region_num(region_num);
  _mpu_set_region_base_addr(base);
  _mpu_set_region_access_ctrl(acc_ctrl);
  _mpu_set_region_size(region_size);

  _mpu_enable_region(region_num, 1);

  return 0;
error:
  printf("_mpu_region_config failed\n");
  return -1;
}
/*---------------------------------------------------------------------------*/
int
mpu_region_config(unsigned region_num, unsigned base, unsigned size, unsigned acc_ctrl)
{
  if(_mpu_region_config(region_num, base, size, acc_ctrl) < 0) {
    goto error;
  }

  if(region_num < MAX_USER_REGION_NUM) {
    if(region_num >= next_empty_region_num) {
      next_empty_region_num = region_num + 1;
    }
  }

  return 0;
error:
  printf("%s failed\n", __func__);
  return -1;
}
/*---------------------------------------------------------------------------*/
int
mpu_get_next_empty_region(void)
{
  return next_empty_region_num;
}
/*---------------------------------------------------------------------------*/
void
mpu_disable_all_user_region(void)
{
  unsigned i;

  for(i = 0; i <= MAX_USER_REGION_NUM; i++) {
    _mpu_enable_region(i, 0);
  }

  next_empty_region_num = 0;
}
/*---------------------------------------------------------------------------*/
void
mpu_disable_user_used_region(void)
{
  unsigned i;

  for(i = 0; i < next_empty_region_num; i++) {
    _mpu_enable_region(i, 0);
  }

  next_empty_region_num = 0;
}
/*---------------------------------------------------------------------------*/
int
mpu_config(unsigned index, unsigned base, unsigned size, unsigned acc_ctrl)
{
  return mpu_region_config(index, base, size, acc_ctrl);
}
/*---------------------------------------------------------------------------*/
void
mpu_clear(unsigned index)
{
  _mpu_enable_region(index, 0);
}
/*---------------------------------------------------------------------------*/
int
mpu_get_next(void)
{
  return mpu_get_next_empty_region();
}
/*---------------------------------------------------------------------------*/
void
mpu_clear_used(void)
{
  mpu_disable_user_used_region();
}
/*---------------------------------------------------------------------------*/
