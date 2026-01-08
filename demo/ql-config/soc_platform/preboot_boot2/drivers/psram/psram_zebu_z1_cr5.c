//cranew cr5 psram no phy psc init code
#include <stdio.h>
#include <stdbool.h>

#include "log.h"

#define ENABLE_MR_INFO_RPINT 0
#define LOG_MODULE "PsramPZ1"
#define LOG_LEVEL LOG_LEVEL_MAIN

#define REG32(addr) (*(volatile unsigned int *)(addr))

/*#define FIX_LATENCY */ /* pz1 and evb no need FIX_LATENCY, haps need it */

#define PSRAM_BASE_NEW  0xC0100000
#define PSRAM_TYPE_NEW  0x1

/*#define FPGA_PLATFORM */

/* #define DIS_PSC_CACHE */ /* zhenbin test disable cache, we need enable cache by self */
#if ENABLE_MR_INFO_RPINT
static int
psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    /* read MR register */
    REG32(PSRAM_BASE_NEW + 0x8034) = mr_addr + (cs << 23);
    REG32(PSRAM_BASE_NEW + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE_NEW + 0x8030);
    }
    read_data = REG32(PSRAM_BASE_NEW + 0x8038);     /* this is the mr read data value */
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8034), REG32(PSRAM_BASE_NEW + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE_NEW + 0x8038), read_data);

    return read_data;
}
#endif

/*---------------------------------------------------------------------------*/
/* type=0 top psc */
/* type=1 mcu psc initial */
/* type=2 mcu psc wakeup */
void
psram_init_zebu_z1_cr5(void)
{
    unsigned read_data;
    /*unsigned mcu_psc; */
    unsigned i = 0;

    LOG_INFO("psram_init_zebu_z1_cr5.\n");
    unsigned psram_base, type;

    psram_base = PSRAM_BASE_NEW;
    type = PSRAM_TYPE_NEW;

    REG32(0xD4015000 + 0x3c) = 0x3;
    for (i = 0; i < 10; i++) {}

#ifndef FPGA_PLATFORM
    /*set psram gpio pin */
    REG32(0xD401e000 + 0x11C) &= ~(0x7);
    REG32(0xD401e000 + 0x11C) |= (0x2);
#endif

    /*mcu_psc = (type==1)||(type==2); */

    REG32(0xd42828f4) = 0x3;

#if 1
    //zhenbin, phyclck=312/2/1=156MHz, 312Mbps;  pcclk=156MHz;
    read_data = REG32(0xd42828b0);
    read_data &= ~(0x7 << 26);
    read_data |= (1 << 31) | (0x5 << 26);
    REG32(0xd42828b0) = read_data;

    read_data = REG32(0xd42828b0);
    while ((read_data & (1 << 31)) == (unsigned)(1 << 31)) {
        read_data = REG32(0xd42828b0);
    }
#endif

    /* reload initial value */
#ifdef WINBOND
    REG32(psram_base + 0x4024) = (0 << 16) | (0 << 12) | (1 << 8) | 0x7;
    REG32(psram_base + 0x8008) |= 0x10;
#else
    REG32(psram_base + 0x4024) = (0 << 16) | (0 << 12) | (0 << 8) | 0x7;
#endif

#ifdef DDR_FPGA_PHY
#ifdef FIX_LATENCY
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
#else
#ifdef NEG_SAMPLE
    REG32(psram_base + 0x18008) = 0xb000;       /*neg, viable */
    REG32(psram_base + 0x19008) = 0xb000;       /*neg, viable */
#else
    REG32(psram_base + 0x18008) = 0x3000;       /*pos, viable */
    REG32(psram_base + 0x19008) = 0x3000;       /*pos, viable */
#endif
#endif
#endif

    REG32(psram_base + 0x18000) = 0x1;
    for (i = 0; i < 20; i++) {
        read_data = REG32(psram_base + 0x18000);
    }
    REG32(psram_base + 0x18000) = 0x5;

#ifdef FIX_LATENCY
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
#endif

#ifdef DIS_PSC_CACHE
    /*disable cache */
    read_data = REG32(psram_base + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(psram_base + 0x4000) = read_data;
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
#ifdef FIX_LATENCY
        /*/ *[REG_C: SEQ_TABLE]* / REG32(psram_base + 0x80d8) = 0x2405442b; */
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054431;
        REG32(psram_base + 0x8030) = 0x104;
        read_data = REG32(psram_base + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(psram_base + 0x8030);
        }
        /*[REG_C: SEQ_TABLE]*/ REG32(psram_base + 0x80d8) = 0x24054433;
#endif
#endif
    }

    REG32(psram_base + 0x0004) = 0x7e690001;

#if ENABLE_MR_INFO_RPINT
    /* initial psc done */
    psram_read_mr(0x0, 0);
    psram_read_mr(0x1, 0);
    psram_read_mr(0x2, 0);
    psram_read_mr(0x3, 0);
    psram_read_mr(0x4, 0);
#endif
    LOG_INFO("PSRAM init done\n");
}
