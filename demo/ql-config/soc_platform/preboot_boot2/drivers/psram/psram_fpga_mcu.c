#include <stdio.h>
#include <stdbool.h>

#include "efuse.h"
#include "cpu.h"
#include "log.h"

#define ENABLE_MR_INFO_RPINT 0

#define LOG_MODULE "PsramFpga"
#define LOG_LEVEL LOG_LEVEL_MAIN

/* MCU_PSRAM 0x3000_0000 32M,
 * connect 2 16M psram sub-chip for fpga test
 */

#define REG32(addr) (*(volatile unsigned int *)(addr))

#define PS_TABLE_DEXC_IDX 21    // 0 disable
#define PS_TABLE_EEXC_IDX 22    // 1 disable

#define PSRAM_DVC_X8

// lijin: 2022_12_02-09:39 MCU psram define
#define PSRAM_DATA_X16      // MCU data width 16

#if defined(PSRAM_DATA_X16) && defined(PSRAM_DVC_X8)
    #define MCU_FIX_LATENCY
#endif

#ifdef  WBD957  // not support hybrid burst
#define WINBOND
#endif  //WBD957

// APM + X16 + single device
#if !defined(WINBOND) && !defined(PSRAM_DVC_X8) && defined(PSRAM_DATA_X16)
    #define APM_Z81P2   //1.2V long lantecy
#endif
// lijin: 2022_12_02-09:39 end



/*
   19:16	rld_type_cfg		RW					"1: APM/WBD 256Mb,
                                    dual CS for MCU, base 0x30000000
                                    0: APM/WBD 64Mb,
                                    dual CS for top, base 0x7e000000"
   15:12	rld_type_dfc		RW					"1: single FP
                                    0: dual FP"
   11:8	rld_type_seq		RW					"1: WBD 64Mb/256Mb
                                    0: APM 64Mb/256Mb
                                    not support yet: APM32Mb, WBD32Mb, UHS"
   7:3	RSV_0			RW
   2	rld_cch_cfg		WO					"initial other psc config
                                    mmap_cfg_cr
                                    cch_config_cr"
   1	rld_dfc_table		WO					"initial dfc table for the init_type
                                    ru_sram
                                    dfc_config_cr"
   0	rld_seq_table		WO					"initial sequence table for the init_type
                                    fp_mr_data_0/1
                                    fp_seq_idx_cr
                                    seq_table
                                    caddr_map
                                    raddr_map"
 */

#define     D_TOP_PSC_INI   0
#define     D_MCU_PSC_INI   1
#define     D_MCU_PSC_WKU   2

#define PSRAM_BASE 0x43000000

#if ENABLE_MR_INFO_RPINT
static int psram_read_mr111(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    // read MR register
    REG32(PSRAM_BASE + 0x8034) = (mr_addr + (cs << 23)) << 1;
    REG32(PSRAM_BASE + 0x8030) = 0x109;
    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }
    read_data = REG32(PSRAM_BASE + 0x8038);     // this is the mr read data value
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8034), REG32(PSRAM_BASE + 0x8034));
    LOG_INFO("RB: MR0x%x_CS%d: @[0x%08X]=[0x%08X]\n", mr_addr, cs, (PSRAM_BASE + 0x8038), read_data);

    return read_data;
}
#endif

// type=0 top psc
// type=1 mcu psc initial
// type=2 mcu psc wakeup
void psram_init_fpga_mcu(void) // lpddr2_400_init
{
    LOG_INFO("psram_init_fpga_mcu for cranew mcu z2!!!\n");

    unsigned read_data;
    unsigned temp_data;
    int i;
    unsigned type = 1;
    unsigned mcu_psc;

    mcu_psc = (type == D_MCU_PSC_INI) || (type == D_MCU_PSC_WKU);

    if (type == D_MCU_PSC_INI) {
        // // lijin: 2022_12_02-09:17 NOTE, may not used for craneW_Z2
        // // enable gpio pad for reset
        // // update to alwayson for resetn and csn pads
        // REG32(0x41200010)		=0x3;
        // for(i=0; i<10; i++){}
        // REG32(0x41207000+97*4)	|= (1<<15)|(1<<14)|0x1;

        //pmu_mpsc_clk_ctrl	0x00B0
        //31:31	MPSC_FLUSH_EN       RW  0x0
        //30:30	MPSC_PU_DLL	        RW  0x0
        //29:29	MPSC_RSTN_UDR       RW  0x0
        //28:28	uclk_sel	        RW  0x0
        //25:25	MPSC_CLK_DFC_REQ	SC  0x0
        //24:24	MPSC_CLK_FC_REQ		SC  0x0
        //23:23	MPSC_CLK_EN		    RW  0x1
        //22:22	MPSC_CLK_RST    	RW  0x1
        //21:21	MPSC_CLK_RAT		RW  0x0
        //20:18	MPSC_CLK_SEL		RW  0x0
        //17:16	MPSC_CLK_DIV		RW  0x0
        //0:2 : MPSC_PLL_EN         RW  0x0

    unsigned mpsc_clk_fc_req = 23;

    if(is_watch1() && hw_rev_id() == REV_ID_WATCH1_A0) {
        mpsc_clk_fc_req=24;
    }

    #if defined(MCU_PSC_HFSPD)
        // 104M
        REG32(0x410000b0) |= (1<<mpsc_clk_fc_req) | (1<<16) ;
        read_data = REG32(0x410000b0);
        while((read_data&(1<<mpsc_clk_fc_req))==(1<<mpsc_clk_fc_req)) {
            read_data = REG32(0x410000b0);
        }
        // lijin: 2022_12_02-20:09 TODO, remove for 208M
    #elif   !defined(WINBOND)   //~MCU_PSC_HFSPD
        // 150M for APM model support only 200MHz
        REG32(0x410000b0) |= (1<<mpsc_clk_fc_req) | (1<<16) | (1<<18) ;
        read_data = REG32(0x410000b0);
        while((read_data&(unsigned)(1<<mpsc_clk_fc_req))==(unsigned)(1<<mpsc_clk_fc_req)) {
            read_data = REG32(0x410000b0);
        }
    #endif //~WINBOND
    }

    if (mcu_psc) {
        // enable clock gating for PSC,MCU_AP reg
        REG32(0x43300040) = 0xff;
    }

    // reload initial value
#ifdef  WINBOND
    REG32(PSRAM_BASE + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (1 << 8) | 0x7;
    // rwds_wndw_en
    REG32(PSRAM_BASE + 0x8008) |= 0x10;
#else   // ~WINBOND
    REG32(PSRAM_BASE + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (0 << 8) | 0x7;
#endif  // ~WINBOND end

    // lijin: 2022_11_25-12:19 add for craneW_Z2, X16 default
    // MCU configuration
    // mmap and for APM16
    // NOTE: default reload MMAP DATA_X16, DVC_X8
#ifdef  PSRAM_DATA_X16
    #ifndef  PSRAM_DVC_X8
    REG32(PSRAM_BASE + 0x4) |= 1 << 6;
    #endif  //~PSRAM_DVC_X8
#else   //~PSRAM_DATA_X16
    REG32(PSRAM_BASE + 0x4) &= ~(3 << 4);
#endif  //~PSRAM_DATA_X16

    if (type != D_MCU_PSC_WKU) {
        // reset device
        // tRP RESET# low pulse width	1us
        // tRST Reset to CMD valid		2us
        REG32(PSRAM_BASE + 0x18000 ) = 0x1;
        for (i = 0; i < 5; i++) {
            read_data = REG32(PSRAM_BASE + 0x18000);                //delay
        }
        REG32(PSRAM_BASE + 0x18000 ) = 0x5;

        if ((type == D_MCU_PSC_INI)) {
            // lijin: 2022_12_02-09:41 TODO, swap no add for craneW_Z2 now
            //#ifdef  PSPAD_SWAP
            //REG32(0x41000000+0xac)	|= (0x1<<28);
            //#endif  // PSPAD_SWAP end
            for (i = 0; i < 20; i++) {
                read_data = REG32(PSRAM_BASE + 0x18000);                // delay for wdt reinit not enough time
            }
        }
        else {
            for (i = 0; i < 40; i++) {
                read_data = REG32(PSRAM_BASE + 0x18000);                // delay
            }
        }
    }

// lijin: 2022_12_02-10:26 NOTE, MCU is single FP!!
#if defined(FIX_LATENCY) || defined(MCU_FIX_LATENCY)
#ifdef  WINBOND
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) |= 1 << (3 + 8);  //LUT4 MRW for low freq
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) |= 1 << 3;        //LUT6 MRW for high freq
    // MR_BYTE, Single FP
    REG32(PSRAM_BASE + 0x8024) |= 1 << (3 + 8);
#else // !WINBOND
      /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) |= 1 << 5; //LUT4, MR0 for high
    // MR_BYTE, Single FP
    REG32(PSRAM_BASE + 0x8024) |= 1 << 5;
#endif  // !WINBOND end
#endif  // FIX_LATENCY end

#ifdef  PSRAM_DATA_X16
#if defined(WBD957)
    //BL128 + legacy burst for MR, read_wrp_dis = 1
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) &= ~(7 << 8); //LUT4 MRW for low freq
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) |= 4 << 8;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) &= ~(7);      //LUT6 MRW for high freq
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) |= 4;

    // MR_BYTE, Single FP
    REG32(PSRAM_BASE + 0x8024) &= ~(7 << 8);
    REG32(PSRAM_BASE + 0x8024) |= 4 << 8;
    //read_wrp_dis
    REG32(PSRAM_BASE + 0x8008) |= 1 << 13;
#elif   defined(WINBOND)
    //BL16
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) &= ~(3 << 8); //LUT4 MRW for low freq
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) |= 2 << 8;
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) &= ~(3);      //LUT6 MRW for high freq
    /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) |= 2;

    // MR_BYTE, Single FP
    REG32(PSRAM_BASE + 0x8024) &= ~(3 << 8);
    REG32(PSRAM_BASE + 0x8024) |= 2 << 8;
#endif  //WINBOND
#endif  //PSRAM_DATA_X16

#ifdef APM_Z81P2
    //update MR for latency
    REG32(PSRAM_BASE + 0x8090 + 0x40 + 8) &= ~(0x7 < 3);    //LUT4 for MR0 high freq, RDlat
    REG32(PSRAM_BASE + 0x8090 + 0x40 + 8) |= 5 << 3;
    REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) &= ~(0x7 < 5);    //LUT6 for MR4 high freq, WRlat
    REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) |= 3 << 5;
    /*[>REG_C, Config PSC*/ /*     fp_mr_data_0[0]<] REG32(PSRAM_BASE + 0x8024) = 0x686017;*/
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) &= ~((0x7 << 21) | (0x7 << 13) | (0x7 << 2));     //~(0xe0e01c);
    /*REG_C, Config PSC*/ /*     fp_mr_data_0[0]*/ REG32(PSRAM_BASE + 0x8024) |= (0x3 << 21) | (0x3 << 13) | (0x5 << 2);        //0x606014;
    //update for APM seq for write latency
    ///*REG_C, Config PSC*/ /*       seq_table[13]*/ REG32(PSRAM_BASE + 0x80c4) = 0x97042008;
    /*REG_C, Config PSC*/ /*       seq_table[13]*/ REG32(PSRAM_BASE + 0x80c4) &= ~(0xf);
    /*REG_C, Config PSC*/ /*       seq_table[13]*/ REG32(PSRAM_BASE + 0x80c4) |= 8;
    //remove bit10 in addr for apmX16
    REG32(PSRAM_BASE + 0x8008) |= 0x8a << 16;
#endif  //APM_Z81P2

//=================================================================================================

#ifdef WINBOND
    // enable phy for winbond
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << (24 + 4);
    REG32(PSRAM_BASE + 0x18004) = read_data;

    // lijin: 2022_12_02-17:03 add for CEN earlier for WBD high speed
    REG32(PSRAM_BASE + 0x14008) &= ~1;
    read_data = REG32(PSRAM_BASE + 0x18004);
    read_data |= 1 << 1;
    REG32(PSRAM_BASE + 0x18004) = read_data;
#endif  // WINBOND end

#ifdef DIS_PSC_CACHE
    //disable cache
    read_data = REG32(PSRAM_BASE + 0x4000);
    read_data = read_data & 0xff0f;
    REG32(PSRAM_BASE + 0x4000) = read_data;
#endif

#ifndef NOT_SIM_SPEED_UP
    // NOTE, only for simulation, mclk is 3M+ instead of 32K, also update model to 1/10
    // 7:4	wait_mp_cycle		RW	0	wait_2 instr count for this+1 mclk as 1 step
    // 3:0	wait_up_cycle		RW	0	wait_1 instr count for this+1 uclk as 1 step
    REG32(PSRAM_BASE + 0x802c) = 0x70;
#endif  // ~NOT_SIM_SPEED_UP end


    //wait for dll status
    // lijin: 2022_12_02-13:21 update for craneW_Z2
    read_data = REG32(PSRAM_BASE + 0x18020 /*0x18010*/);
    while ((read_data & 0x100) != 0x100) {
        read_data = REG32(PSRAM_BASE + 0x18020 /*0x18010*/);
    }

    if (type == D_MCU_PSC_WKU) {
        // exit from half sleep
        REG32(PSRAM_BASE + 0x8030) = 0x108;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
    #ifndef WINBOND
        // exit from slow refresh
    #ifdef  PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x4 << 1;
    #else   //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x4;
    #endif  //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8030) = 0x10c;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
    #endif  //~WINBOND
    }
    else {  //~D_MCU_PSC_WKU
    #ifdef  WINBOND
        //TODO, WBD soft reset!
    #else   // ~WINBOND
        // global reset for if initial for wdt reset
        REG32(PSRAM_BASE + 0x8030) = 0x105;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
    #endif  // ~WINBOND end

    #ifdef WINBOND
        // MR for high speed
        // 1. hybird read
        // 2. fix latency or variable latency
        // 3. burst length for X16
    #ifdef  PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x1000 << 1;
    #else   //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x1000;
    #endif  //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8030) = 0x106;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
    #else // !WINBOND
        // MR for high speed
        // MR0
        REG32(PSRAM_BASE + 0x8030) = 0x104;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
        // MR4
    #ifdef  PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x4 << 1;
    #else   //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x4;
    #endif  //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8030) = 0x106;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
        // MR8 for BL16 if datawidh=16
    #ifdef  PSRAM_DATA_X16
        read_data = REG32(PSRAM_BASE + 0x8090 + 0x60 + 4);
        temp_data = read_data;
        read_data &= ~(0xff);
    #ifndef PSRAM_DVC_X8
        read_data |= 0x044; //X16 and BL16
    #else                   //PSRAM_DVC_X8
        read_data |= 0x04;  //X8 and BL16
    #endif  //PSRAM_DVC_X8
        REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) = read_data;
    #ifdef  PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x8 << 1;
    #else   //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8034) = 0x8;
    #endif  //~PSRAM_DATA_X16
        REG32(PSRAM_BASE + 0x8030) = 0x106;
        read_data = REG32(PSRAM_BASE + 0x8030);
        while ((read_data & 0x100) != 0x0) {
            read_data = REG32(PSRAM_BASE + 0x8030);
        }
        REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) = temp_data;
    #endif  //PSRAM_DATA_X16
    #endif  // !WINBOND
    }//~D_MCU_PSC_WKU
#if ENABLE_MR_INFO_RPINT
    psram_read_mr111(0x0, 0);
    psram_read_mr111(0x1, 0);
    psram_read_mr111(0x2, 0);
    psram_read_mr111(0x3, 0);
    psram_read_mr111(0x4, 0);
#endif
}
