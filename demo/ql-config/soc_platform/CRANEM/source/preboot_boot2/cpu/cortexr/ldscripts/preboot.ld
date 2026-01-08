__min_heap_size = 3K;
__stack_size = 0;

/*
 * btcm space *
 * ptable      :  btcm 0 K ~ 3 K, length 3 K
 * asr property:  btcm 3 K ~ 4 K, length 1 K
 * user defined:  btcm 4 K ~ 32K, length 28K
 * bootrom     :  btcm 32K ~ 64K, length 32K
 */
__btcm_user_offset = 4K;

INCLUDE cortexr_base.ld
