#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"
#include "log.h"
#define LOG_MODULE "PsramNoPhy"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define REG32(addr) (*(volatile unsigned int *)(addr))
#define PSRAM_BASE  (0xc0100000)
#define PSRAM_TYPE  0x1
#define DIS_PSC_CACHE_NO_PHY

static void psram_init_zebu_or_fpga_no_phy(int fix_latency, int is_fpga);

/*---------------------------------------------------------------------------*/
void
psram_init_fpga_no_phy(void)
{
    if (hw_platform_type() == HW_PLATFORM_TYPE_FPGA) {
        psram_init_zebu_or_fpga_no_phy(1, 1);
    }
}

void
psram_init_zebu_no_phy(void)
{
    if (hw_platform_type() == HW_PLATFORM_TYPE_ZEBU_Z1) {
        psram_init_zebu_or_fpga_no_phy(0, 0);
    }
}

/* type=0 top psc */
/* type=1 mcu psc initial */
/* type=2 mcu psc wakeup */
static void
psram_init_zebu_or_fpga_no_phy(int fix_latency, int is_fpga)
{
    unsigned read_data;
    unsigned mcu_psc;
    unsigned i = 0;

    unsigned psram_base, type;

    psram_base = PSRAM_BASE;
    type = PSRAM_TYPE;

    LOG_INFO("Psram init ---- no phy psc init!!!\n");

    REG32(0xD4015000 + 0x3c) = 0x3;
    for (i = 0; i < 10; i++) {}

    if (!is_fpga) {
        /*set psram gpio pin */
        REG32(0xD4090000 + 0x114) = 0x75;
        REG32(0xD4090000 + 0x128) = 0x75757575;
    }

    mcu_psc = (type == 1) || (type == 2);

    REG32(0xd42828f4) = 0x3;

    read_data = REG32(0xd42828b0);
    read_data &= ~(0x7 << 26);
    read_data |= (1 << 31) | (0x5 << 26);
    REG32(0xd42828b0) = read_data;

    read_data = REG32(0xd42828b0);
    while ((read_data & (1 << 31)) == (unsigned)(1 << 31)) {
        read_data = REG32(0xd42828b0);
    }

    /* reload initial value */
#ifdef WINBOND
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (1 << 8) | 0x7;
    REG32(psram_base + 0x8008) |= 0x10;
#else
    REG32(psram_base + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (0 << 8) | 0x7;
#endif

#ifdef DDR_FPGA_PHY
    if (fix_latency) {
#ifdef WINBOND
#ifdef NEG_SAMPLE
        REG32(psram_base + 0x18008) = 0xc7ff;           /*neg, fix */
        REG32(psram_base + 0x19008) = 0xc7ff;           /*neg, fix */
#else
        REG32(psram_base + 0x18008) = 0x47ff;           /*pos, fix */
        REG32(psram_base + 0x19008) = 0x47ff;           /*pos, fix */
#endif
#else
#ifdef NEG_SAMPLE
        REG32(psram_base + 0x18008) = 0xf000;           /*neg, fix */
        REG32(psram_base + 0x19008) = 0xf000;           /*neg, fix */
#else
        REG32(psram_base + 0x18008) = 0xf1ff;           /*pos, viable */
        REG32(psram_base + 0x19008) = 0xf1ff;           /*pos, viable */
        REG32(psram_base + 0x1a008) = 0xf1ff;           /*pos, viable */
        REG32(psram_base + 0x1b008) = 0xf1ff;           /*pos, viable */
#endif
#endif
    }
    else {
#ifdef NEG_SAMPLE
        REG32(psram_base + 0x18008) = 0xb000;           /*neg, viable */
        REG32(psram_base + 0x19008) = 0xb000;           /*neg, viable */
#else
        REG32(psram_base + 0x18008) = 0x3000;           /*pos, viable */
        REG32(psram_base + 0x19008) = 0x3000;           /*pos, viable */
#endif
    }
#endif

    REG32(psram_base + 0x18000) = 0x1;
    for (i = 0; i < 20; i++) {
        read_data = REG32(psram_base + 0x18000);
    }
    REG32(psram_base + 0x18000) = 0x5;

    if (fix_latency) {
#ifdef WINBOND
        /* low freq */
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80dc) = 0x840bc49f;
        /* high freq */
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80f8) = 0x2402842b;
        REG32(psram_base + 0x8024) = 0x0b9f;
        REG32(psram_base + 0x9024) = 0x2b9f;
#else
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
        REG32(psram_base + 0x8024) = 0x402b;
        REG32(psram_base + 0x9024) = 0x2033;
#endif
    }

#ifdef DIS_PSC_CACHE_NO_PHY
    /*disable cache */
    read_data = REG32(psram_base + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(psram_base + 0x4000) = read_data;
    LOG_INFO("psc cache disable!!!  \n");
#else
    LOG_INFO("psc cache enable!!!  \n");
#endif

#ifdef WINBOND
    /* enable phy for winbond */
    read_data = REG32(psram_base + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(psram_base + 0x18004) = read_data;
#endif

    if (type == 2) {
        /* exit from half sleep */
        REG32(psram_base + 0x8030) = 0x108;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
    }
    else {
#ifdef WINBOND
        /* // lijin: 2021_08_23-11:53 TODO cs1, temp read mr for if wdt reset initial, ?? */
        /* REG32(PSRAM_BASE + 0x8034) = 0x0; */
        /* REG32(PSRAM_BASE + 0x8030) = 0x109; */
        /* read_data = REG32(PSRAM_BASE + 0x8030) ; */
        /* while ((read_data & 0x100)!=0x0) { */
        /* read_data = REG32(PSRAM_BASE + 0x8030) ; */
        /* } */
#else
        /* global reset for if initial for wdt reset */
        REG32(psram_base + 0x8030) = 0x105;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
#endif

#ifdef WINBOND
        /* program MR for */
        /* 1. hybird read */
        /* 2. fix latency or variable latency */
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
#else
        if (fix_latency) {
            /*/ *[REG_C: SEQ_TABLE]* / REG32(psram_base + 0x80d8) = 0x2405442b; */
            /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054431;
            REG32(psram_base + 0x8030) = 0x104;
            read_data = REG32(psram_base + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(psram_base + 0x8030);
            }
            /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
        }
#endif
    }

    REG32(psram_base + 0x0004) = 0x7e690001;
} /* init_psram end */
/*---------------------------------------------------------------------------*/
