/*
 * Copyright (c) 2020, ASR micro Incorporated
 * All rights reserved.
 */

#ifndef _REG_H_
#define _REG_H_

#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* low level macros for accessing memory mapped hardware registers */
#define REG64(addr) ((volatile uint64_t *)(addr))
#define REG32(addr) ((volatile uint32_t *)(addr))
#define REG16(addr) ((volatile uint16_t *)(addr))
#define REG8(addr)  ((volatile uint8_t *)(addr))

#define RMWREG64(addr, startbit, width, val) *REG64(addr) = (*REG64(addr) & ~(((1 << (width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG32(addr, startbit, width, val) *REG32(addr) = (*REG32(addr) & ~(((1 << (width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG16(addr, startbit, width, val) *REG16(addr) = (*REG16(addr) & ~(((1 << (width)) - 1) << (startbit))) | ((val) << (startbit))
#define RMWREG8(addr, startbit, width, val) *REG8(addr) = (*REG8(addr) & ~(((1 << (width)) - 1) << (startbit))) | ((val) << (startbit))

#define writel(v, a)   (*REG32(a) = (v))
#define readl(a)       (*REG32(a))

#define writeb(v, a)   (*REG8(a) = (v))
#define readb(a)       (*REG8(a))

#define writehw(v, a)  (*REG16(a) = (v))
#define readhw(a)      (*REG16(a))

#define writew(v, a)   (*REG16(a) = (v))
#define readw(a)       (*REG16(a))

static inline void
__raw_writesl(unsigned long addr, const void *data, int longlen)
{
  uint32_t *buf = (uint32_t *)data;

  while(longlen--) {
    writel(*buf++, addr);
  }
}
static inline void
__raw_readsl(unsigned long addr, void *data, int longlen)
{
  uint32_t *buf = (uint32_t *)data;

  while(longlen--) {
    *buf++ = readl(addr);
  }
}
/**
 * @def SETBIT
 * @brief Sets a bitmask for a bitfield
 *
 * @param[in] val   The bitfield
 * @param[in] bit   Specifies the bits to be set
 *
 * @return The modified bitfield
 */
#define SETBIT(val, bit)    val |= (bit)

/**
 * @def CLRBIT
 * @brief Clears bitmask for a bitfield
 *
 * @param[in] val   The bitfield
 * @param[in] bit   Specifies the bits to be cleared
 *
 * @return The modified bitfield
 */
#define CLRBIT(val, bit)    val &= (~(bit))

#ifndef BIT0
#define BIT0  0x00000001
#define BIT1  0x00000002
#define BIT2  0x00000004
#define BIT3  0x00000008
#define BIT4  0x00000010
#define BIT5  0x00000020
#define BIT6  0x00000040
#define BIT7  0x00000080
#define BIT8  0x00000100
#define BIT9  0x00000200
#define BIT10 0x00000400
#define BIT11 0x00000800
#define BIT12 0x00001000
#define BIT13 0x00002000
#define BIT14 0x00004000
#define BIT15 0x00008000
#endif
#ifndef BIT16
#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000
#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000
#endif

#ifdef __cplusplus
extern "C" {
#endif

#endif /* _REG_H_ */
