#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned int crc32(unsigned int val, const void *ss, int len);

#ifdef __cplusplus
}
#endif

#endif /* CRC32_H */
