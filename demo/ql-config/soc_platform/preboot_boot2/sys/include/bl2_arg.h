#ifndef __BL2_ARG_H_
#define __BL2_ARG_H_

#include <stdint.h>

#define BL2_ARG_DETECT_SIZE 128
#define BL2_ARG_MAGIC       0x424C3241
#define BL2_ARG_VERSION     0x1

typedef struct bl2_arg {
  uint32_t magic;
  uint32_t version;
  uint32_t disable_log;
} bl2_arg_t;

int bl2_arg_init(bl2_arg_t *arg);
bl2_arg_t *bl2_arg_get(void);

#endif
