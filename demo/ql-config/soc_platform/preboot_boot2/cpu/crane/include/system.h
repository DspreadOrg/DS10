#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*---------------------------------------------------------------------------*/
#define MPU_REGION_ACCESS_CTRL_TEX(x) (((x) & 0x7) << 3)    /* Type Extensions */
#define MPU_REGION_ACCESS_CTRL_AP(x)  (((x) & 0x7) << 8)    /* Access Permissions */
/*---------------------------------------------------------------------------*/
#define isb() __asm__ volatile ("isb" : : : "memory")
#define dsb() __asm__ volatile ("dsb" : : : "memory")
/*---------------------------------------------------------------------------*/
#define SCTLR_M    (1 << 0)     /* Enable the MPU */
#define SCTLR_A    (1 << 1)     /* Alignment check enable bit */
#define SCTLR_C    (1 << 2)     /* Cache enable bit */
#define SCTLR_SW   (1 << 10)    /* SWP/SWPB enable bit */
#define SCTLR_Z    (1 << 11)    /* Branch prediction enable bit */
#define SCTLR_I    (1 << 12)    /* Instruction cache enable bit */
#define SCTLR_V    (1 << 13)    /* Vectors relocated to 0xffff0000 */
#define SCTLR_RR   (1 << 14)    /* Not on Cortex-R7 processor. Round Robin bit for cache implementation policy */
#define SCTLR_BR   (1 << 17)    /* Background region bit */
#define SCTLR_DZ   (1 << 19)    /* Divide by zero fault bit */
#define SCTLR_FI   (1 << 21)    /* Fast interrupts configuration enable */
#define SCTLR_U    (1 << 22)    /* Not on Cortex-R7 processor. Indicates use of the alignment model */
#define SCTLR_EE   (1 << 25)    /* Exception endianness */
#define SCTLR_NMFI (1 << 27)    /* Non-maskable FIQ(NMFI) support */
#define SCTLR_TE   (1 << 30)    /* Thumb exception enable */
#define SCTLR_IE   (1 << 31)    /* Instruction Endianness */
/*---------------------------------------------------------------------------*/
static inline unsigned int
sctlr_get(void)
{
  unsigned int val;

  __asm__ __volatile__ ("mrc p15, 0, %0, c1, c0, 0" : "=r" (val) : : "memory", "cc");

  return val;
}
/*---------------------------------------------------------------------------*/
static inline void
sctlr_set(unsigned int val)
{
  __asm__ __volatile__ ("mcr p15, 0, %0, c1, c0, 0" : : "r" (val) : "memory", "cc");
  isb();
}
/*---------------------------------------------------------------------------*/
#endif /* _SYSTEM_H_ */
