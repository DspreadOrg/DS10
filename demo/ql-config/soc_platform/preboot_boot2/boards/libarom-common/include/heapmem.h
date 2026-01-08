/* -*- C -*- */
/*
 * Copyright (c) 2020, ASR microelectronics
 * All rights reserved.
 */

#ifndef HEAPMEM_H
#define HEAPMEM_H

#include <stdlib.h>

typedef struct heapmem_stats {
  size_t allocated;
  size_t overhead;
  size_t available;
  size_t footprint;
  size_t chunks;
} heapmem_stats_t;

void *arom_heapmem_alloc(size_t size);
void *arom_heapmem_realloc(void *ptr, size_t size);
void arom_heapmem_free(void *ptr);

void *heapmem_alloc(size_t size);
void *heapmem_realloc(void *ptr, size_t size);
void heapmem_free(void *ptr);

#endif /* HEAPMEM_H */
/** @} */
