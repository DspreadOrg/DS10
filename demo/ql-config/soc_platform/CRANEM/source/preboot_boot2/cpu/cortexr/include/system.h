#ifndef _SYSTEM_H_
#define _SYSTEM_H_

/*---------------------------------------------------------------------------*/
#define MPU_REGION_ACCESS_CTRL_TEX(x) (((x) & 0x7) << 3)    /* Type Extensions */
#define MPU_REGION_ACCESS_CTRL_AP(x)  (((x) & 0x7) << 8)    /* Access Permissions */

#define MODE32_svc              0x13
#define SPSR_T_ARM              0
#define SPSR_E_LITTLE           0
#define SPSR_FIQ_BIT            (1 << 0)
#define SPSR_IRQ_BIT            (1 << 1)
#define SPSR_ABT_BIT            (1 << 2)
#define DISABLE_ALL_EXCEPTIONS  (SPSR_FIQ_BIT | SPSR_IRQ_BIT | SPSR_ABT_BIT)

#define MODE_RW_SHIFT           0x4
#define MODE_RW_32              0x1
#define MODE32_SHIFT            0
#define MODE32_MASK             0x1f
#define SPSR_T_SHIFT            5
#define SPSR_T_MASK             0x1
#define SPSR_E_SHIFT            9
#define SPSR_E_MASK             0x1
#define SPSR_AIF_SHIFT          6
#define SPSR_AIF_MASK           0x7
/*---------------------------------------------------------------------------*/
#define SPSR_MODE32(mode, isa, endian, aif) \
  (MODE_RW_32 << MODE_RW_SHIFT | \
   ((mode) & MODE32_MASK) << MODE32_SHIFT | \
   ((isa) & SPSR_T_MASK) << SPSR_T_SHIFT | \
   ((endian) & SPSR_E_MASK) << SPSR_E_SHIFT | \
   ((aif) & SPSR_AIF_MASK) << SPSR_AIF_SHIFT)

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
