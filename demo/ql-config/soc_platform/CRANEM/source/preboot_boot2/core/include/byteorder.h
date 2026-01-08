#ifndef BYTEORDER_H
#define BYTEORDER_H

#include "types.h"

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif

#define __bitwise __bitwise__

typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

#define uswap_16(x) \
  ((((x) & 0xff00) >> 8) | \
   (((x) & 0x00ff) << 8))
#define uswap_32(x) \
  ((((x) & 0xff000000) >> 24) | \
   (((x) & 0x00ff0000) >> 8) | \
   (((x) & 0x0000ff00) << 8) | \
   (((x) & 0x000000ff) << 24))
#define _uswap_64(x, sfx) \
  ((((x) & 0xff00000000000000 ## sfx) >> 56) | \
   (((x) & 0x00ff000000000000 ## sfx) >> 40) | \
   (((x) & 0x0000ff0000000000 ## sfx) >> 24) | \
   (((x) & 0x000000ff00000000 ## sfx) >> 8) | \
   (((x) & 0x00000000ff000000 ## sfx) << 8) | \
   (((x) & 0x0000000000ff0000 ## sfx) << 24) | \
   (((x) & 0x000000000000ff00 ## sfx) << 40) | \
   (((x) & 0x00000000000000ff ## sfx) << 56))

#define uswap_64(x) _uswap_64(x, ull)

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x)     (x)
#define cpu_to_le32(x)     (x)
#define cpu_to_le64(x)     (x)
#define le16_to_cpu(x)     (x)
#define le32_to_cpu(x)     (x)
#define le64_to_cpu(x)     (x)
#define cpu_to_be16(x)     uswap_16(x)
#define cpu_to_be32(x)     uswap_32(x)
#define cpu_to_be64(x)     uswap_64(x)
#define be16_to_cpu(x)     uswap_16(x)
#define be32_to_cpu(x)     uswap_32(x)
#define be64_to_cpu(x)     uswap_64(x)
#define be32_to_cpup(x)    uswap_32(*(x))
#else
#define cpu_to_le16(x)     uswap_16(x)
#define cpu_to_le32(x)     uswap_32(x)
#define cpu_to_le64(x)     uswap_64(x)
#define le16_to_cpu(x)     uswap_16(x)
#define le32_to_cpu(x)     uswap_32(x)
#define le64_to_cpu(x)     uswap_64(x)
#define cpu_to_be16(x)     (x)
#define cpu_to_be32(x)     (x)
#define cpu_to_be64(x)     (x)
#define be16_to_cpu(x)     (x)
#define be32_to_cpu(x)     (x)
#define be64_to_cpu(x)     (x)
#endif

#ifndef __packed
#define __packed    __attribute__((__packed__))
#endif

#ifndef ALIGN
#define ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#endif

#endif /*BYTEORDER_H*/
