#ifndef _EFUSE_H
#define _EFUSE_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern bool is_cranels(void);
extern bool is_cranelr(void);
extern bool is_cranew(void);
extern bool is_cranelg(void);
extern bool is_cranelrh(void);
extern bool is_cranelb(void);
extern bool is_watch1(void);

/*
 * define fuse value for psram devie
 * PSRAM Type Code:
 * AP 16M                        00000
 * Winbond 16M                   00001
 * AP 8M + 8M                    00010
 * Winbond 8M + 8M               00100
 * AP 8M                         00101
 * Winbond 8M                    00110
 * Winbond 250MHz 8M             00111
 * AP UHS 8M + UHS 8M            01000
 * Winbond 250MHz 8M + 250MHz 8M 01001
 */
#define AP_16M               0
#define WB_16M               1
#define AP_8M8M              2
#define UNIIC_16M            3
#define WB_8M8M              4
#define AP_8M                5
#define WB_8M                6
#define WB_250MHZ_8M         7
#define AP_UHS_8M8M          8
#define WB_250MHZ_8M8M       9
#define AP_250MHZ_8M8M       10
#define WB_XCCELA_8M         11
#define WB_XCCELA_8M8M       12
#define WB_4M                13
#define AP_4M                14
#define WB_XCCELA_4M         15
#define AP_250MHZ_8M         16
#define WB_32M               17
#define AP_250MHZ_16M        18
#define WB_250MHZ_16M        19
#define UNIIC_4M             20

typedef enum {
  NO_PHY_AP_4M,             /* 0 */
  NO_PHY_WB_4M,             /* 1 */
  NO_PHY_AP_8M,             /* 2 */
  NO_PHY_WB_8M,             /* 3 */
  NO_PHY_AP_16M,            /* 4 */
  NO_PHY_WB_16M,            /* 5 */
  NO_PHY_UNIIC_16M,         /* 6 */
  NO_PHY_UNIIC_4M,          /* 7 */
  NO_PHY_PSRAM_TYPE_MAX     /* 8 */
} NO_PHY_PSRAM_TYPE_T;

#define AP_16M_NAME         "AP_16M"            /* 0 */
#define WB_16M_NAME         "WB_16M"            /* 1 */
#define AP_8M8M_NAME        "AP_8M8M"           /* 2 */
#define WB_8M8M_NAME        "WB_8M8M"           /* 4 */
#define AP_8MB_NAME         "AP_8M"             /* 5 */
#define WB_8MB_NAME         "WB_8M"             /* 6 */
#define WB_250MHZ_8M_NAME   "WB_250MHZ_8M"      /* 7 */
#define AP_UHS_8M8M_NAME    "AP_UHS_8M8M"       /* 8 */
#define WB_250MHZ_8M8M_NAME "WB_250MHZ_8M8M"    /* 9 */
#define AP_250MHZ_8M8M_NAME "AP_250MHZ_8M8M"    /* 10 */
#define WB_XCCELA_8M_NAME   "WB_XCCELA_8M"      /* 11 */
#define WB_XCCELA_8M8M_NAME "WB_XCCELA_8M8M"    /* 12 */
#define WB_4MB_NAME         "WB_4M"             /* 13 */
#define AP_4MB_NAME         "AP_4M"             /* 14 */
#define WB_XCCELA_4M_NAME   "WB_XCCELA_4M"      /* 15 */
#define AP_250MHZ_8M_NAME   "AP_250MHZ_8M"      /* 16 */
#define WB_32M_NAME         "WB_32M"            /* 17 */
#define AP_250MHZ_16M_NAME  "AP_250MHZ_16M"     /* 18 */
#define WB_250MHZ_16M_NAME  "WB_250MHZ_16M"     /* 19 */
#define UNIIC_16M_NAME      "UNIIC_16M"         /* 20 */
#define UNIIC_4M_NAME       "UNIIC_4M"          /* 21 */

/* psram_crane_mcu.h defined psram type id instead of efuse.h */
#define AP256Mb_1_2V_X16_CS0_NAME        "AP256Mb_1_2V_X16_CS0"           /* 0 */
#define AP256Mb_1_2V_X16_CS1_NAME        "AP256Mb_1_2V_X16_CS1"           /* 1 */
#define AP256Mb_1_2V_X16_CS0_CS1_NAME    "AP256Mb_1_2V_X16_CS0_CS1"       /* 2 */
#define WB256Mb_1_8V_X16_CS0_NAME        "WB256Mb_1_8V_X16_CS0"           /* 3 */
#define WB256Mb_1_8V_X16_CS1_NAME        "WB256Mb_1_8V_X16_CS1"           /* 4 */
#define WB256Mb_1_8V_X16_CS0_CS1_NAME    "WB256Mb_1_8V_X16_CS0_CS1"       /* 5 */


#define PSRAM_SIZE_4M             0x400000
#define PSRAM_SIZE_8M             0x800000
#define PSRAM_SIZE_16M            0x1000000
#define PSRAM_SIZE_32M            0x2000000

/* software defined ddr type, can diff with fuse value for ddr device:
 * DDR Type Code:
 * CraneGT use DDR， connect flash instead of SIP
 * 3605S： DDR 32MB + Nor flash 16MB
 * 3605E： DDR 64MB + Nand flash 64MB
 */
typedef enum {
  DRAM_AP_32M,              /* 0 / * 3605S-111 * / */
  DRAM_UniIC_32M,           /* 1 / * 3605S-110 * / */
  DRAM_AP_64M,              /* 2 / * 3605E-110 * / */
  DSRAM_TYPE_MAX            /* 3 */
} DRAM_TYPE_T;

#define DRAM_AP_32M_NAME        "DRAM_AP_32M" /*Bytes */
#define DRAM_AP_64M_NAME        "DRAM_AP_64M"
#define DRAM_UniIC_32M_NAME     "DRAM_UniIC_32M"
#define DRAM_UniIC_64M_NAME     "DRAM_UniIC_64M"


/* reversed fuse ddr bits, but not use fuse now, use autodetect */
#define DRAM_SIZE_32M            0x2000000
#define DRAM_SIZE_64M            0x4000000
#define DRAM_SIZE_128M           0x8000000

unsigned int efuse_get_psram_version(void);
unsigned efuse_cranew_mcu_psram(void);
unsigned int get_psram_size(void);
unsigned int get_ddr_size(void);
void fuse_read_lotid(void);
int fuse_trust_boot_enabled(void);
bool is_chip_support_auto_detect(void);

#ifdef __cplusplus
}
#endif

#endif /* _EFUSE_H */
/** @} */
