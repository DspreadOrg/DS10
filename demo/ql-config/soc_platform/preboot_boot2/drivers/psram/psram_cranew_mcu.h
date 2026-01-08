#ifndef _PSRAM_CRANEW_MCU_H_
#define _PSRAM_CRANEW_MCU_H_

//FUSE USE THIS!!!
#define     AP256Mb_1_2V_X16_CS0        0
#define     AP256Mb_1_2V_X16_CS1        1
#define     AP256Mb_1_2V_X16_CS0_CS1    2
#define     WB256Mb_1_8V_X16_CS0        3
#define     WB256Mb_1_8V_X16_CS1        4
#define     WB256Mb_1_8V_X16_CS0_CS1    5



#define     PSRAM_BASE      0x43000000

#define     AP256Mb_1_2V_X16        0
#define     WB256Mb_1_8V_X16        1

#define     CS0                     0
#define     CS1                     1
#define     CS0_AND_CS1             2
#define     CS_ERR                  3

enum {
    WBD_DRIVE_34ohm_default = 0,
    WBD_DRIVE_115ohm,
    WBD_DRIVE_67ohm,
    WBD_DRIVE_46ohm,
    WBD_DRIVE_34ohm,
    WBD_DRIVE_27ohm,
    WBD_DRIVE_22ohm,
    WBD_DRIVE_19ohm,
};

enum {
    APM_X16_1_2V_DRIVE_25ohm,
    APM_X16_1_2V_DRIVE_50ohm,
    APM_X16_1_2V_DRIVE_100ohm,
    APM_X16_1_2V_DRIVE_200ohm,
};

enum {
    PAD_DRIVE_18V_HIGH_Z= 0,
    PAD_DRIVE_18V_400ohm,
    PAD_DRIVE_18V_300ohm,
    PAD_DRIVE_18V_240ohm,
    PAD_DRIVE_18V_200ohm,
    PAD_DRIVE_18V_100ohm,
    PAD_DRIVE_18V_50ohm_1,
    PAD_DRIVE_18V_50ohm_2,
};

enum {
    PAD_DRIVE_12V_HIGH_Z= 0,
    PAD_DRIVE_12V_440ohm,
    PAD_DRIVE_12V_360ohm,
    PAD_DRIVE_12V_288ohm,
    PAD_DRIVE_12V_240ohm,
    PAD_DRIVE_12V_120ohm,
    PAD_DRIVE_12V_60ohm_1,
    PAD_DRIVE_12V_60ohm_2,
};

//#define PSRAM_AUTO_TRAINING    //Sv Debug use, preboot don't care this!!!

#endif
