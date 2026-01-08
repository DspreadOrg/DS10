__min_heap_size = 3K;
__stack_size = 0;

/*
 * btcm space *
 * ptable      :  btcm 0 K ~ 3 K, length 3 K
 * asr property:  btcm 3 K ~ 4 K, length 1 K
 * lzma decode :  btcm 4 K ~ 20K, length 16K
 * user defined:  btcm 20K ~ 32K, length 12K
 * bootrom     :  btcm 32K ~ 64K, length 32K
 */
__btcm_user_offset = 20K;

/**
 * psram space when boot *
 * boot33 lzma decode space: 0x7e000000 ~ 0x7e100000, length 1M
 * boot33 load space       : 0x7e100000 ~ 0x7e200000, length 1M
 */
#include "crane_base.lds"
