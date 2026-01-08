#include "psram.h"
#include "cpu.h"
#include "property.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "efuse.h"
#include "assert.h"

#include "log.h"

/*
 *  This is mcu psram init,and it can auto detect psram type.So psram fuse is not must.
 *
 *  Support mcu psram type:
 *  WB_X16_1.8V_CS0/WB_X16_1.8V_CS1/WB_X16_1.8V_CS0&CS1
 *  AP_X16_1.2V_CS0/AP_X16_1.2V_CS1/AP_X16_1.2V_CS0&CS1
 *
 *	If mcu psram add new type, please contact sv first for the latest init file.
 */
#include "psram_cranew_mcu.h"

#define LOG_MODULE "PsramMcu"
#define LOG_LEVEL LOG_LEVEL_MAIN
#undef libEFUSE

#define REG32(addr) (*(volatile unsigned int *)(addr))

#define     D_TOP_PSC_INI   0   // type=0 top psc
#define     D_MCU_PSC_INI   1   // type=1 mcu psc initial
#define     D_MCU_PSC_WKU   2   // type=2 mcu psc wakeup

static unsigned mcu_psc_hfspd = 0;
static unsigned psram_data_x16 = 1;
static unsigned psram_dvc_x8 = 0;
static unsigned psram_cs = 0;
static unsigned pspad_swap = 0;
static unsigned fix_latency = 0;
static unsigned mcu_fix_latency = 0;
static unsigned wbd957 = 0;
static unsigned dis_psc_cache = 0;
static unsigned sdf_on = 0;
static unsigned WB256Mb_1_2V_X16_DRIVE = 0;
static unsigned APM256Mb_1_2V_X16_DRIVE = 0;
static unsigned version = 0;
static unsigned auto_detect_cnt = 0;

static const char *psram_type_use[2][3] = {
    { AP256Mb_1_2V_X16_CS0_NAME, AP256Mb_1_2V_X16_CS1_NAME, AP256Mb_1_2V_X16_CS0_CS1_NAME },
    { WB256Mb_1_8V_X16_CS0_NAME, WB256Mb_1_8V_X16_CS1_NAME, WB256Mb_1_8V_X16_CS0_CS1_NAME }
};

#if AROM_VERSION == AROM_VER_2022_07_28_CRANEW_Z2
static const unsigned psram_type_fuse_id[2][3] = {
    { AP256Mb_1_2V_X16_CS0, AP256Mb_1_2V_X16_CS1, AP256Mb_1_2V_X16_CS0_CS1 },
    { WB256Mb_1_8V_X16_CS0, WB256Mb_1_8V_X16_CS1, WB256Mb_1_8V_X16_CS0_CS1 }
};
#endif


#ifdef libEFUSE
static unsigned psram_fuse(void)
{
    /*craneW: Bit 177~173 */
    unsigned version_info = efuse_cranew_mcu_psram();

    char psram_str[PROP_VALUE_MAX] = "";

    LOG_INFO("########################\n");
    LOG_INFO("Version ID : %d\n", version_info);
    switch (version_info) {
        case AP256Mb_1_2V_X16_CS0:
            version = AP256Mb_1_2V_X16;
            psram_cs = CS0;
            memcpy(psram_str, AP256Mb_1_2V_X16_CS0_NAME, PROP_VALUE_MAX);
            break;
        case AP256Mb_1_2V_X16_CS1:
            version = AP256Mb_1_2V_X16;
            psram_cs = CS1;
            memcpy(psram_str, AP256Mb_1_2V_X16_CS1_NAME, PROP_VALUE_MAX);
            break;
        case AP256Mb_1_2V_X16_CS0_CS1:
            version = AP256Mb_1_2V_X16;
            psram_cs = CS0_AND_CS1;
            memcpy(psram_str, AP256Mb_1_2V_X16_CS0_CS1_NAME, PROP_VALUE_MAX);
            break;
        case WB256Mb_1_8V_X16_CS0:
            version = WB256Mb_1_8V_X16;
            psram_cs = CS0;
            memcpy(psram_str, WB256Mb_1_8V_X16_CS0_NAME, PROP_VALUE_MAX);
            break;
        case WB256Mb_1_8V_X16_CS1:
            version = WB256Mb_1_8V_X16;
            psram_cs = CS1;
            memcpy(psram_str, WB256Mb_1_8V_X16_CS1_NAME, PROP_VALUE_MAX);
            break;
        case WB256Mb_1_8V_X16_CS0_CS1:
            version = WB256Mb_1_8V_X16;
            psram_cs = CS0_AND_CS1;
            memcpy(psram_str, WB256Mb_1_8V_X16_CS0_CS1_NAME, PROP_VALUE_MAX);
            break;
        default:
            LOG_PRINT("Unknow Device\n");
            assert(0);
            break;
    }

    LOG_PRINT("%s\n", psram_str);
    asr_property_set("fuse.psram.type", psram_str);

    LOG_INFO("########################\n");
    return version_info;
}
#else
static void set_psram_property_autodetect(unsigned version, unsigned psram_cs)
{
    char psram_str[PROP_VALUE_MAX] = "";

    LOG_INFO("########################\n");
    if (version > WB256Mb_1_8V_X16 || psram_cs >= CS_ERR) {
        LOG_PRINT("Unknow Device\n");
        assert(0);
    }
    else {
        memcpy(psram_str, psram_type_use[version][psram_cs], PROP_VALUE_MAX);

#if AROM_VERSION == AROM_VER_2022_07_28_CRANEW_Z2
        //MCU_SW_DEBUG_REG bit 7~0, store psram type fuse id for sv, the register data will not lost after mcu reset
        REG32(MCU_AON_BASE + MCU_SW_DEBUG_REG) = (REG32(MCU_AON_BASE + MCU_SW_DEBUG_REG) & (~0xff)) |
                                                     (psram_type_fuse_id[version][psram_cs] & 0xff);
#endif
    }

    LOG_PRINT("%s\n", psram_str);
    asr_property_set("fuse.psram.type", psram_str);

    LOG_INFO("########################\n");
}
#endif
static void set_drive_ohm(void)
{
    if (version == WB256Mb_1_8V_X16) {
        WB256Mb_1_2V_X16_DRIVE = WBD_DRIVE_115ohm;
    }

    if (version == AP256Mb_1_2V_X16) {
        APM256Mb_1_2V_X16_DRIVE = APM_X16_1_2V_DRIVE_100ohm;
    }
}

void pad_drive_str_config(void)
{
    unsigned data = 0;
    unsigned pad_dqs_str = 0;
    unsigned pad_csn_str = 0;
    unsigned pad_clk_str = 0;
    unsigned pad_adq_str = 0;

    if (version == AP256Mb_1_2V_X16) {
        pad_dqs_str = PAD_DRIVE_12V_120ohm;
        pad_csn_str = PAD_DRIVE_12V_120ohm;
        pad_clk_str = PAD_DRIVE_12V_120ohm;
        pad_adq_str = PAD_DRIVE_12V_120ohm;
    }
    else {
        pad_dqs_str = PAD_DRIVE_18V_100ohm;
        pad_csn_str = PAD_DRIVE_18V_100ohm;
        pad_clk_str = PAD_DRIVE_18V_100ohm;
        pad_adq_str = PAD_DRIVE_18V_100ohm;
    }

    data = (pad_dqs_str << 24) | (pad_csn_str << 16) | (pad_clk_str << 8) | (pad_adq_str << 0);
    REG32(PSRAM_BASE + 0x18010) = data;

    LOG_INFO(" @[0x%08X]=[0x%08X]\n", (PSRAM_BASE + 0x18010), REG32(PSRAM_BASE + 0x18010));
}

static void set_config_for_chip(void)
{
    if (is_watch1()) {
        mcu_psc_hfspd = 0;
        psram_data_x16 = 1;
        psram_dvc_x8 = 0;
        pspad_swap = 0;
        fix_latency = 0;
        mcu_fix_latency = 0;
        wbd957 = 0;
        dis_psc_cache = 0;
        sdf_on = 0;
    }

    if (is_cranew()) {
        mcu_psc_hfspd = 0;
        psram_data_x16 = 1;
        psram_dvc_x8 = 0;
        pspad_swap = 0;
        fix_latency = 0;
        mcu_fix_latency = 0;
        wbd957 = 0;
        dis_psc_cache = 0;
        sdf_on = 0;
    }
}

static unsigned mcu_psram_read_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    // read MR register
    REG32(PSRAM_BASE + 0x8034) = (mr_addr + (cs << 24)) << 1;
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

unsigned mcu_psram_write_mr(unsigned mr_addr, unsigned cs)
{
    unsigned read_data = 0;

    // read MR register
    REG32(PSRAM_BASE + 0x8034) = (mr_addr + (cs << 24)) << 1;
    REG32(PSRAM_BASE + 0x8030) = 0x10b;

    read_data = REG32(PSRAM_BASE + 0x8030);
    while ((read_data & 0x100) != 0x0) {
        read_data = REG32(PSRAM_BASE + 0x8030);
    }

    return read_data;
}

void ap_1_2v_psram_refresh_to_1x(unsigned cs)
{
    unsigned reg_data = 0;

    reg_data = REG32(0x43000000 + 0x8024);
    reg_data &= ~((0x3 << 11) | (0x3 << 19));
    reg_data |= ((0x1 << 11) | (0x1 << 19));
    REG32(0x43000000 + 0x8024) = reg_data;

    if ((cs == CS0) || (cs == CS1)) {
        mcu_psram_write_mr(0x4, cs); //refresh to 1x
    }
    else {
        mcu_psram_write_mr(0x4, 0); //refresh to 1x
        mcu_psram_write_mr(0x4, 1); //refresh to 1x
    }
}
unsigned auto_detect_psram_init(unsigned version)
{

    unsigned read_data = 0, i = 0;
    unsigned mcu_psc = 0;
    unsigned mpsc_clk_fc_req = 23;
    unsigned type = D_MCU_PSC_INI;

    set_config_for_chip();

    set_drive_ohm();


    mcu_psc = (type == D_MCU_PSC_INI) || (type == D_MCU_PSC_WKU);

    if (type == D_MCU_PSC_INI) {
        if (version == WB256Mb_1_8V_X16) {
            //enable gpio pad for reset, update to alwayson for resetn and csn pads
            REG32(0x41200010) = 0x3;
            for (i = 0; i < 10; i++) {}
            REG32(0x41207000 + 97 * 4) |= (1 << 15) | (1 << 14) | 0x1; //MGPIO52 FOR MCU_PSRAM_RST
        }

        LOG_INFO("mpsc_clk_fc_req:0x%x\n", mpsc_clk_fc_req);
        if (is_watch1() && hw_rev_id() == REV_ID_WATCH1_A0) {
            mpsc_clk_fc_req = 24;
        }

        /*pmu_mpsc_clk_ctrl	0x00B0 */
        //31:31	MPSC_FLUSH_EN       RW  0x0
        //30:30	MPSC_PU_DLL	        RW  0x0
        //29:29	MPSC_RSTN_UDR       RW  0x0
        //28:28	uclk_sel	        RW  0x0
        //25:25	MPSC_CLK_DFC_REQ	SC  0x0
        //24:24	MPSC_CLK_FC_REQ		SC  0x0
        //23:23	MPSC_CLK_EN		    RW  0x1
        //22:22	MPSC_CLK_RST        RW  0x1
        //21:21	MPSC_CLK_RAT		RW  0x0
        //20:18	MPSC_CLK_SEL		RW  0x0
        //17:16	MPSC_CLK_DIV		RW  0x0
        //0:2 : MPSC_PLL_EN         RW  0x0

        if (mcu_psc_hfspd) {
            // 104M
            REG32(0x410000b0) |= (1 << mpsc_clk_fc_req) | (1 << 16);
            read_data = REG32(0x410000b0);
            while ((read_data & (unsigned)(1 << mpsc_clk_fc_req)) == (unsigned)(1 << mpsc_clk_fc_req)) {
                read_data = REG32(0x410000b0);
            }
        }
        else {
            if (version == AP256Mb_1_2V_X16) {
                // 150M for APM model support only 200MHz
                REG32(0x410000b0) |= (1 << mpsc_clk_fc_req) | (1 << 16) | (1 << 18);
                read_data = REG32(0x410000b0);
                while ((read_data & (unsigned)(1 << mpsc_clk_fc_req)) == (unsigned)(1 << mpsc_clk_fc_req)) {

                    read_data = REG32(0x410000b0);
                }
            }
        }
    }

    if (mcu_psc) {
        // enable clock gating for PSC,MCU_AP reg
        REG32(0x43300040) = 0xff;
    }

    if (psram_data_x16) {
        if (psram_dvc_x8) {
            REG32(PSRAM_BASE + 0x0004) = 0x3009001D;  //32M byte
        }
        else {
            if (psram_cs == CS0) {//cs0
                REG32(PSRAM_BASE + 0x0004) = 0x3009005D;
            }
            else if (psram_cs == CS1) {//cs1
                REG32(PSRAM_BASE + 0x0004) = 0x3009085D;
            }
            else { //dula cs
                REG32(PSRAM_BASE + 0x0004) = 0x300A605D;
            }
        }
    }

    // reload initial value
    if (version == WB256Mb_1_8V_X16) {
        REG32(PSRAM_BASE + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (1 << 8) | 0x3;
        REG32(PSRAM_BASE + 0x8008) |= 0x10;  // rwds_wndw_en
    }
    else {
        REG32(PSRAM_BASE + 0x4024) = (mcu_psc << 16) | (mcu_psc << 12) | (0 << 8) | 0x3;
    }

    if (psram_data_x16) {
        if (!psram_dvc_x8) {
            REG32(PSRAM_BASE + 0x4) |= 1 << 6;
            if (version == WB256Mb_1_8V_X16) {
                REG32(PSRAM_BASE + 0x8088) = 0x97969594;
                REG32(PSRAM_BASE + 0x808c) = 0x9b9a0098;
            }
        }
    }
    else {
        REG32(PSRAM_BASE + 0x4) &= ~(3 << 4);
    }


    if (type != D_MCU_PSC_WKU) {
        // reset device
        // tRP RESET# low pulse width	1us
        // tRST Reset to CMD valid		2us
        REG32(PSRAM_BASE + 0x18000 ) = 0x1;
        for (i = 0; i < 5; i++) {
            read_data = REG32(PSRAM_BASE + 0x18000);                    //delay
        }
        REG32(PSRAM_BASE + 0x18000 ) = 0x5;

        if ((type == D_MCU_PSC_INI)) {
            if (pspad_swap) {
                REG32(0x41000000 + 0xac) |= (0x1 << 28);
            }
            for (i = 0; i < 20; i++) {
                read_data = REG32(PSRAM_BASE + 0x18000);                 //delay for wdt reinit not enough time
            }
        }
        else {
            for (i = 0; i < 40; i++) {
                read_data = REG32(PSRAM_BASE + 0x18000);                 // delay
            }
        }
    }

    if (fix_latency || mcu_fix_latency) {
        if (version == WB256Mb_1_8V_X16) {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) |= 1 << (3 + 8);  //LUT4 MRW for low freq
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) |= 1 << 3;        //LUT6 MRW for high freq
            REG32(PSRAM_BASE + 0x8024) |= 1 << (3 + 8);                         // MR_BYTE, Single FP
        }
        else {
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80d8) |= 1 << 5;    //LUT4, MR0 for high
            REG32(PSRAM_BASE + 0x8024) |= 1 << 5;                           // MR_BYTE, Single FP
        }
    }

    if (psram_data_x16) {
        if (wbd957) {
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
        }
        else if (version == WB256Mb_1_8V_X16) {
            //LUT4 MRW for low freq
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80dc) = (0xc403c69f);

            //LUT6 MRW for high freq
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f4) = (0xc48fcf10) | (WB256Mb_1_2V_X16_DRIVE << 20);
            /*[REG_C: SEQ_TABLE]*/ REG32(PSRAM_BASE + 0x80f8) = 0x2402c422;

            // MR_BYTE, Single FP
            REG32(PSRAM_BASE + 0x8024) &= ~(3 << 8);
            REG32(PSRAM_BASE + 0x8024) |= 2 << 8;
        }
    }

    if (version == AP256Mb_1_2V_X16) {
        // LUT2: RD 200!
        REG32(PSRAM_BASE + 0x80b0) = 0x8b200400;
        REG32(PSRAM_BASE + 0x80b4) = 0x9b04200a;
        REG32(PSRAM_BASE + 0x80b8) = 0x0;
        // LUT3: WR 200!
        REG32(PSRAM_BASE + 0x80c0) = 0x8b200480;
        REG32(PSRAM_BASE + 0x80c4) = 0x9704200b;
        REG32(PSRAM_BASE + 0x80c8) = 0x0;
        //LUT4 for MR0 high freq, RDlat
        REG32(PSRAM_BASE + 0x80d0) = 0xc4c0c4c0;
        REG32(PSRAM_BASE + 0x80d4) = 0x44004400;
        REG32(PSRAM_BASE + 0x80d8) = 0x2402441c | APM256Mb_1_2V_X16_DRIVE;
        REG32(PSRAM_BASE + 0x80dc) = 0x0;
        //LUT6 for MR4 high freq, WRlat
        REG32(PSRAM_BASE + 0x80f0) = 0xcb2044c0;
        REG32(PSRAM_BASE + 0x80f4) = 0x24094498; //0.5x refrsh
        REG32(PSRAM_BASE + 0x80f8) = 0x28022c00;
        REG32(PSRAM_BASE + 0x80fc) = 0x2c03;

        REG32(PSRAM_BASE + 0x8024) &= 0xff000000;
        REG32(PSRAM_BASE + 0x8024) |= 0x0098981c | APM256Mb_1_2V_X16_DRIVE;

        //remove bit10 in addr for apmX16
        REG32(PSRAM_BASE + 0x8008) |= 0x8a << 16;
    }

    // lijin: 2023_02_06-16:59 for pipe setting in sdc for timing for craneW
    REG32(PSRAM_BASE + 0x14008) &= ~1;
    read_data = REG32(PSRAM_BASE + 0x18004);

    if (version == WB256Mb_1_8V_X16) {
        // enable phy for winbond
        read_data = REG32(PSRAM_BASE + 0x18004);
        read_data |= 1 << (24 + 4);
        REG32(PSRAM_BASE + 0x18004) = read_data;

        //// lijin: 2022_12_02-17:03 add for CEN earlier for WBD high speed, pipe setting
        read_data = REG32(PSRAM_BASE + 0x18004);
        read_data |= 1 << 1;
        REG32(PSRAM_BASE + 0x18004) = read_data;
    }

    if (dis_psc_cache) {
        //disable cache
        read_data = REG32(PSRAM_BASE + 0x4000);
        read_data = read_data & 0xff0f;
        REG32(PSRAM_BASE + 0x4000) = read_data;
        LOG_INFO("Psram cache disabled !!!\n ");
    }
    else {
        LOG_INFO("Psram cache enabled !!!\n ");
    }

    //wait for dll status
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

        if (version == AP256Mb_1_2V_X16) {
            // exit from slow refresh
            if (psram_data_x16) {
                REG32(PSRAM_BASE + 0x8034) = 0x4 << 1;
            }
            else {
                REG32(PSRAM_BASE + 0x8034) = 0x4;
            }

            REG32(PSRAM_BASE + 0x8030) = 0x10c;
            read_data = REG32(PSRAM_BASE + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(PSRAM_BASE + 0x8030);
            }
        }
    }
    else {
        if (version == WB256Mb_1_8V_X16) {
            //TODO, WBD soft reset!
        }
        else {
            // global reset for if initial for wdt reset
            REG32(PSRAM_BASE + 0x8030) = 0x105;
            read_data = REG32(PSRAM_BASE + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(PSRAM_BASE + 0x8030);
            }
        }

        //temp enable oen 1/2 ui pre/pst for dq
        if (sdf_on) {
            read_data = REG32(PSRAM_BASE + 0x18004);
            read_data |= (0x1 << 5) | (0x1 << 3);
            REG32(PSRAM_BASE + 0x18004) = read_data;
        }

        if (version == WB256Mb_1_8V_X16) {
            // MR for high speed
            // 1. hybird read
            // 2. fix latency or variable latency
            // 3. burst length for X16
            if (psram_data_x16) {
                REG32(PSRAM_BASE + 0x8034) = 0x1000 << 1;
            }
            else {
                REG32(PSRAM_BASE + 0x8034) = 0x1000;
            }

            REG32(PSRAM_BASE + 0x8030) = 0x106;
            read_data = REG32(PSRAM_BASE + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(PSRAM_BASE + 0x8030);
            }
        }
        else {
            // MR for high speed
            // MR0
            REG32(PSRAM_BASE + 0x8030) = 0x104;
            read_data = REG32(PSRAM_BASE + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(PSRAM_BASE + 0x8030);
            }

            // MR4
            if (psram_data_x16) {
                REG32(PSRAM_BASE + 0x8034) = 0x4 << 1;
            }
            else {
                REG32(PSRAM_BASE + 0x8034) = 0x4;
            }

            REG32(PSRAM_BASE + 0x8030) = 0x106;
            read_data = REG32(PSRAM_BASE + 0x8030);
            while ((read_data & 0x100) != 0x0) {
                read_data = REG32(PSRAM_BASE + 0x8030);
            }

            // MR8 for BL16 if datawidh=16
            if (psram_data_x16) {
                unsigned temp_data;
                read_data = REG32(PSRAM_BASE + 0x8090 + 0x60 + 4);
                temp_data = read_data;
                read_data &= ~(0xff);

                if (!psram_dvc_x8) {
                    read_data |= 0x044;  //X16 and BL16
                }
                else {
                    read_data |= 0x04;  //X8 and BL16
                }

                REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) = read_data;

                if (psram_data_x16) {
                    REG32(PSRAM_BASE + 0x8034) = 0x8 << 1;
                }
                else {
                    REG32(PSRAM_BASE + 0x8034) = 0x8;
                }

                REG32(PSRAM_BASE + 0x8030) = 0x106;
                read_data = REG32(PSRAM_BASE + 0x8030);
                while ((read_data & 0x100) != 0x0) {
                    read_data = REG32(PSRAM_BASE + 0x8030);
                }
                REG32(PSRAM_BASE + 0x8090 + 0x60 + 4) = temp_data;
            }
        }
    }

    unsigned mr_data0 = 0, mr_data1 = 0, mr_data2 = 0, mr_data3 = 0;

    if (auto_detect_cnt == 0) {
        if (version == WB256Mb_1_8V_X16) {
            mr_data0 = mcu_psram_read_mr(0x0, 0);
            mr_data1 = mcu_psram_read_mr(0x0, 1);
            if ((mr_data0 == 0x0076000E) && (mr_data1 == 0x0076000E)) {
                psram_cs = CS0_AND_CS1;
            }
            else if ((mr_data0 != 0x0076000E) && (mr_data1 == 0x0076000E)) {
                psram_cs = CS1;
            }
            else if ((mr_data0 == 0x0076000E) && (mr_data1 != 0x0076000E)) {
                psram_cs = CS0;
            }
            else {
                LOG_INFO("Not WB256Mb_1_8V_X16!!!\n");
                psram_cs = CS_ERR;
            }
        }
        else {
            mr_data0 = mcu_psram_read_mr(0x1, 0) & 0xff;
            mr_data1 = mcu_psram_read_mr(0x2, 0) & 0xff;
            mr_data2 = mcu_psram_read_mr(0x1, 1) & 0xff;
            mr_data3 = mcu_psram_read_mr(0x2, 1) & 0xff;

            if ((mr_data0 == 0x8D) && (mr_data1 == 0xC7) && (mr_data2 == 0x8D) && (mr_data3 == 0xC7)) {
                psram_cs = CS0_AND_CS1;
            }
            else if (((mr_data0 != 0x8D) || (mr_data1 != 0xC7)) && ((mr_data2 == 0x8D) && (mr_data3 == 0xC7))) {
                psram_cs = CS1;
            }
            else if (((mr_data0 == 0x8D) || (mr_data1 == 0xC7)) && ((mr_data2 != 0x8D) && (mr_data3 != 0xC7))) {
                psram_cs = CS0;
            }
            else {
                LOG_INFO("Not AP256Mb_1_2V_X16!!!\n");
                psram_cs = CS_ERR;
            }
        }
    }

    auto_detect_cnt++;

    return psram_cs;
}

void psc_phy_clk_rst_then_release(void)
{
    unsigned mpsc_clk_reset = 21;
    unsigned reg = 0, i = 0;

    if (is_watch1() && hw_rev_id() == REV_ID_WATCH1_A0) {
        mpsc_clk_reset = 22;
    }

    //reset psc_clk and then release
    reg = REG32(0xd42828b0);
    reg &= ~(1 << mpsc_clk_reset);
    REG32(0xd42828b0) = reg;
    for (i = 0; i < 1000; i++) {//delay
    }
    reg |= (1 << mpsc_clk_reset);
    REG32(0xd42828b0) = reg;
}

void psram_init_cranew_mcu(void)
{
    unsigned ret = 0;

    LOG_INFO("Mcu psram init begin!!!\n");
    /*get version information */
#ifdef libEFUSE
    psram_fuse();
    LOG_INFO("MCU PSRAM INIT BY FUSE!!! VERSION:%s \n", psram_type_use[version][psram_cs]);
    auto_detect_psram_init(version);
#else
    LOG_INFO("Begin to auto detect,first try AP256Mb_1_2V_X16!!!\n");
    version = AP256Mb_1_2V_X16;
    psram_cs = CS0_AND_CS1;
    ret = auto_detect_psram_init(version);
    if (ret == CS_ERR) {
        LOG_INFO("AP256Mb_1_2V_X16 init fail, Begin to try WB256Mb_1_8V_X16!!!\n");
        psc_phy_clk_rst_then_release();
        version = WB256Mb_1_8V_X16;
        psram_cs = CS0_AND_CS1;
        ret = auto_detect_psram_init(version);
        if (ret == CS_ERR) {
            LOG_INFO("WB256Mb_1_8V_X16 init fail, This type psram haven't support now!!!\n");
            return;
        }
        else {
            psc_phy_clk_rst_then_release();
            version = WB256Mb_1_8V_X16;
            psram_cs = ret;
            auto_detect_psram_init(version);
            LOG_INFO("MCU PSRAM VERSION:%s \n", psram_type_use[version][psram_cs]);
        }
    }
    else {
        psc_phy_clk_rst_then_release();
        version = AP256Mb_1_2V_X16;
        psram_cs = ret;
        auto_detect_psram_init(version);
        LOG_INFO("MCU PSRAM VERSION:%s \n", psram_type_use[version][psram_cs]);
    }

    set_psram_property_autodetect(version, psram_cs);

#endif

    pad_drive_str_config();

    //Sv debug use, preboot don't care these code!!!
#if PSRAM_AUTO_TRAINING
    void psram_dqs_dly_training(void);
    psram_dqs_dly_training();
    return;
#endif

    //enbale pre and post for dq/dm
    REG32(0x43010000 + 0x8004) |= (0x1 << 3) | (0x1 << 5);
    REG32(0x43010000 + 0x9004) |= (0x1 << 3) | (0x1 << 5);

    REG32(0x43010000 + 0x8004) &= ~(0xf << 16);
    REG32(0x43010000 + 0x9004) &= ~(0xf << 16);
    REG32(0x43010000 + 0x800c) &= ~(0xf << 16);
    REG32(0x43010000 + 0x900c) &= ~(0xf << 16);
    REG32(0x43010000 + 0x8004) |= (0xc << 16);
    REG32(0x43010000 + 0x9004) |= (0xc << 16);
    REG32(0x43010000 + 0x800c) |= (0xc << 16);
    REG32(0x43010000 + 0x900c) |= (0xc << 16);

    REG32(0x43010000 + 0x8008) &= ~(0xff << 0);
    REG32(0x43010000 + 0x8008) |= (0xa7 << 0);
    REG32(0x43010000 + 0x9008) &= ~(0xff << 0);
    REG32(0x43010000 + 0x9008) |= (0xa7 << 0);

    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x8004, REG32(0x43010000 + 0x8004));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x9004, REG32(0x43010000 + 0x9004));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x800c, REG32(0x43010000 + 0x800c));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x900c, REG32(0x43010000 + 0x900c));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x8008, REG32(0x43010000 + 0x8008));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x9008, REG32(0x43010000 + 0x9008));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x4010, REG32(0x43010000 + 0x4010));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x8020, REG32(0x43010000 + 0x8020));

    if ((version == WB256Mb_1_8V_X16) && (psram_cs == CS0)) {
        mcu_psram_read_mr(0x0, 0);
        mcu_psram_read_mr(0x2, 0);
        mcu_psram_read_mr(0x1000, 0);
        mcu_psram_read_mr(0x1002, 0);
    }
    else if ((version == WB256Mb_1_8V_X16) && (psram_cs == CS1)) {
        mcu_psram_read_mr(0x0, 0);
        mcu_psram_read_mr(0x2, 0);
        mcu_psram_read_mr(0x1000, 0);
        mcu_psram_read_mr(0x1002, 0);
    }
    else if ((version == WB256Mb_1_8V_X16) && (psram_cs == CS0_AND_CS1)) {
        mcu_psram_read_mr(0x0, CS0);
        mcu_psram_read_mr(0x2, CS0);
        mcu_psram_read_mr(0x1000, CS0);
        mcu_psram_read_mr(0x1002, CS0);

        mcu_psram_read_mr(0x0, CS1);
        mcu_psram_read_mr(0x2, CS1);
        mcu_psram_read_mr(0x1000, CS1);
        mcu_psram_read_mr(0x1002, CS1);
    }
    else if ((version == AP256Mb_1_2V_X16) && (psram_cs == CS0)) {
        ap_1_2v_psram_refresh_to_1x(psram_cs);

        mcu_psram_read_mr(0x0, 0);
        mcu_psram_read_mr(0x1, 0);
        mcu_psram_read_mr(0x2, 0);
        mcu_psram_read_mr(0x3, 0);
        mcu_psram_read_mr(0x4, 0);
    }
    else if ((version == AP256Mb_1_2V_X16) && (psram_cs == CS1)) {
        ap_1_2v_psram_refresh_to_1x(psram_cs);

        mcu_psram_read_mr(0x0, 0);
        mcu_psram_read_mr(0x1, 0);
        mcu_psram_read_mr(0x2, 0);
        mcu_psram_read_mr(0x3, 0);
        mcu_psram_read_mr(0x4, 0);
    }
    else if ((version == AP256Mb_1_2V_X16) && (psram_cs == CS0_AND_CS1)) {
        ap_1_2v_psram_refresh_to_1x(psram_cs);

        mcu_psram_read_mr(0x0, CS0);
        mcu_psram_read_mr(0x1, CS0);
        mcu_psram_read_mr(0x2, CS0);
        mcu_psram_read_mr(0x3, CS0);
        mcu_psram_read_mr(0x4, CS0);

        mcu_psram_read_mr(0x0, CS1);
        mcu_psram_read_mr(0x1, CS1);
        mcu_psram_read_mr(0x2, CS1);
        mcu_psram_read_mr(0x3, CS1);
        mcu_psram_read_mr(0x4, CS1);
    }

    LOG_PRINT("Mcu psram init end!!!\n");
}//init_psram_mcu




/*********************Below is Sv debug code, preboot don't care!!!*********************/
#ifdef PSRAM_AUTO_TRAINING
#define DLL_CODE_MODE //default
#if defined(DLL_CODE_MODE)
unsigned dqs_dly_training(void)
{
    LOG_PRINT("dqs_dly_training begin !!!  dll_code mode!!! \n\r");

    unsigned dqs_dly[16] = { 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7 };

    //enbale pre and post for dq/dm
    REG32(0x43010000 + 0x8004) |= (0x1 << 3) | (0x1 << 5);
    REG32(0x43010000 + 0x9004) |= (0x1 << 3) | (0x1 << 5);

    // set delay line:
    unsigned i = 0;

    REG32(0x43010000 + 0x8004) &= ~(0xf << 16);
    REG32(0x43010000 + 0x9004) &= ~(0xf << 16);
    REG32(0x43010000 + 0x800c) &= ~(0xf << 16);
    REG32(0x43010000 + 0x900c) &= ~(0xf << 16);
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x8004, REG32(0x43010000 + 0x8004));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x9004, REG32(0x43010000 + 0x9004));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x800c, REG32(0x43010000 + 0x800c));
    LOG_PRINT("dll@[0x%X]=[0x%x]  \n", 0x43010000 + 0x900c, REG32(0x43010000 + 0x900c));

    extern uint32 mem_all_pattern_test(unsigned long uiStartBase, uint32 uiSize);
    unsigned ret = FAIL;
    unsigned start_addr = 0x30000000;
    unsigned scan_size = 0x400000;

    for (i = 0x0; i < 0x10; i++) {
        //byte0
        REG32(0x43010000 + 0x8004) &= ~(0xf << 16);
        REG32(0x43010000 + 0x8004) |= (dqs_dly[i] << 16);
        REG32(0x43010000 + 0x9004) &= ~(0xf << 16);
        REG32(0x43010000 + 0x9004) |= (dqs_dly[i] << 16);

        //byte1
        REG32(0x43010000 + 0x800c) &= ~(0xf << 16);
        REG32(0x43010000 + 0x800c) |= (dqs_dly[i] << 16);
        REG32(0x43010000 + 0x900c) &= ~(0xf << 16);
        REG32(0x43010000 + 0x900c) |= (dqs_dly[i] << 16);

        ret = mem_all_pattern_test(start_addr, scan_size);

        LOG_PRINT("###dll@[0x43018004]=[0x%x],@[0x43019004]=[0x%x],@[0x4301800c]=[0x%x],@[0x4301900c]=[0x%x], ret=[%d]###\n", REG32(0x43010000 + 0x8004), REG32(0x43010000 + 0x9004), REG32(0x43010000 + 0x800c), REG32(0x43010000 + 0x900c), ret);
    }

    LOG_PRINT("dqs_dly_training end !!!	\n\r");

    return 0;
}
#endif

#if defined(DELAYLINE_MODE)
unsigned dqs_dly_training(void)
{
    LOG_PRINT("dqs_dly_training begin !!!  delayline mode!!!\n\r");

    //enbale pre and post for dq/dm
    REG32(0x43010000 + 0x8004) |= (0x1 << 3) | (0x1 << 5);
    REG32(0x43010000 + 0x9004) |= (0x1 << 3) | (0x1 << 5);

    unsigned delay_step = 0x0;

    REG32(0x43010000 + 0x8008) &= ~(0x3 << 8);
    REG32(0x43010000 + 0x9008) &= ~(0x3 << 8);
    REG32(0x43010000 + 0x8008) |= (delay_step << 8);
    REG32(0x43010000 + 0x9008) |= (delay_step << 8);

    REG32(0x43010000 + 0x8008) &= ~(0x1 << 12);
    REG32(0x43010000 + 0x9008) &= ~(0x1 << 12);
    REG32(0x43010000 + 0x8008) |= (0x1 << 12);
    REG32(0x43010000 + 0x9008) |= (0x1 << 12);

    // set delay line:
    unsigned i = 0;

    extern uint32 mem_all_pattern_test(unsigned long uiStartBase, uint32 uiSize);
    unsigned ret = FAIL;
    unsigned start_addr = 0x30000000;
    unsigned scan_size = 0x40000;

    for (i = 0x0; i < 0x100; i++) {
        REG32(0x43010000 + 0x8008) &= ~(0xff << 16);
        REG32(0x43010000 + 0x9008) &= ~(0xff << 16);
        REG32(0x43010000 + 0x8008) |= (i << 16);
        REG32(0x43010000 + 0x9008) |= (i << 16);

        REG32(0x43010000 + 0x800c) &= ~(0xff << 24);
        REG32(0x43010000 + 0x900c) &= ~(0xff << 24);
        REG32(0x43010000 + 0x800c) |= (i << 24);
        REG32(0x43010000 + 0x900c) |= (i << 24);

        ret = mem_all_pattern_test(start_addr, scan_size);

        LOG_PRINT("###dll@[0x43018008]=[0x%x],@[0x43019008]=[0x%x],@[0x4301800c]=[0x%x],@[0x4301900c]=[0x%x], ret=[%d]###\n", REG32(0x43010000 + 0x8008), REG32(0x43010000 + 0x9008), REG32(0x43010000 + 0x800c), REG32(0x43010000 + 0x900c), ret);
    }

    LOG_PRINT("dqs_dly_training end !!!	\n\r");

    return 0;
}
#endif

extern void dclk_dfc_fulmar_mcu_psram(unsigned pscclk_level);
extern void dclk_dfc_watch1_mcu_psram(unsigned phyclk_level, unsigned pscclk_sel);

void psram_dqs_dly_training(void)
{
    unsigned vcc_main = 0;

    vcc_main = get_vcc_main(0);
    LOG_PRINT("@@@@@@@@before set vcc_main:%d mv !!!	\n\r", vcc_main / 1000);
    vcc_main = 1000000;
    set_vcc_main(vcc_main, 0);
    vcc_main = get_vcc_main(0);
    LOG_PRINT("@@@@@@@@after set vcc_main:%d mv !!!	\n\r", vcc_main / 1000);

    if (is_fulmar()) {
        dclk_dfc_fulmar_mcu_psram(2);
    }

    if (is_watch1()) {
        dclk_dfc_watch1_mcu_psram(2, 0);
    }

    dqs_dly_training();
}
#endif
