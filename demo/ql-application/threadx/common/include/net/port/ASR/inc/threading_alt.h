
#ifndef MBEDTLS_THREADING_ALT_H
#define MBEDTLS_THREADING_ALT_H

#include "mbedtls/threading.h"

extern void mbedtls_port_threading_mutex_init(mbedtls_threading_mutex_t * mutex);
extern void mbedtls_port_threading_mutex_free(mbedtls_threading_mutex_t * mutex);
extern int mbedtls_port_threading_mutex_lock(mbedtls_threading_mutex_t * mutex);
extern int mbedtls_port_threading_mutex_unlock(mbedtls_threading_mutex_t * mutex);
extern void mbedtls_port_threading_exit(int status);

#endif

