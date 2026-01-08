#ifndef __PMIC_H__
#define __PMIC_H__

#if defined(BOARD_CRANE) || defined(BOARD_CRANEW_MCU)

#include <stddef.h>
#include <stdint.h>

#define PMIC_BASE_PAGE_ADDR                       0x30    /* base page 7-bit i2c address */
/* base page register address */
#define PMIC_ID                                   0x0
#define PM813_MISC_CFG_REG1                       0xd
#define PM813_PWRUP_LOG_REG1                      0x10
#define PMIC_WATCHDOG                             0x1d
#define WATCHDOG_WD_DIS                           (1 << 0) /* disable watchdog when set */
#define PM813_PRE_CHARGE_CURRENT_SETTING_REG      0x21
#define PRE_CHARGE_CURRENT_SETTING_150MA          (3 << 2)/* 11 bit3~2 */

/* only pm803 bug: if read or wirte  0xE7, 0xE5 and 0xE6(shutdown reason) are cleared automatically */
#define PM813_FAULT_EVENT_REG1                    0xe5
#define PM813_FAULT_EVENT_REG2                    0xe6

#define PM813_FAULT_WU_REG                        0xe7
#define PM813_RTC_SPARE_REG1                      0xe9
#define PM802_RTC_SPARE_REG1                      0xe8
#define PM802S_RTC_SPARE_REG1                     0xcf
#define PM816_RTC_SPARE_REG1                      0xc8
#define PM813L_RTC_SPARE_REG1                     0xc8
#define PM807_RTC_SPARE_REG1                      0xcf
#define PM802_FAULT_WU_ENABLE_BIT                 (1 << 2)
#define PM813_FAULT_WU_ENABLE_BIT                 (1 << 0)
#define PM813_FAULT_WU_BIT                        (1 << 3)
#define PM813S_FAULT_WU_BIT                       (1 << 1)
#define PM813_SW_PDOWN                            (1 << 5)
#define PM813_RTC_REG_XO                          0xf0
#define CRYSTAL_10PF                              (1 << 5)/* 001 bit7~5 */

#define PMIC_POWER_PAGE_ADDR                      0x31    /* base page 7-bit i2c address */
/* power page register address */
#define PM813_BUCK1_VOLTAGE_SET_REG               0x20
#define PM813_DVC_SET_REG                         0x21
#define PM816_BUCK1_VOLTAGE_SET_REG               0x30
#define PM816_FSM_BUCK1_REG2                      0x36
#define PM816_FSM_BUCK1_GO2PWM                    (1 << 4)

#define PM816_DVC_SET_REG                         0x41
#define DVC_ENALE                                 (1 << 7)

#define PM813_LDO11_REG                           0x8F
#define LDO11_ENABLE                              (1 << 6)
#define LDO11_VSET_1_2V_BIT                       (0xF << 2)
#define LDO11_VSET_1_2V                           (0x0 << 2)

#define PM803_FSM_BUCK1_REG2                      0x22
#define PM803_FSM_BUCK1_GO2PWM                    (1 << 5)

#define PM803L_FSM_BUCK1_REG5                     0x25
#define PM803L_FSM_BUCK1_GO2PWM                   (1 << 0)

#define PM807_FSM_BUCK1_REG3                      0x23
#define PM807_FSM_BUCK1_GO2PWM                    (1 << 0)

#define PM813L_FSM_BUCK1_REG2                     0x26
#define PM813L_FSM_BUCK1_GO2PWM                   (1 << 0)
#define PM813L_BUCK1_VOLTAGE_SET_REG              0x20
#define PM813L_DVC_SET_REG                        0x31
#define PM813L_RTC_REG_XO                         0xf5
#define CRYSTAL_10PF_813L                         (3 << 0)/* 011 bit2~0 */

#define PM803_LDO7_SLEEP_MODE_REG                 0x82
#define PM803_LDO7_ACTIVE_MODE_REG                0x83
#define LDO7_VSET_1_6V_BIT                        (0xF)
#define LDO7_VSET_1_6V                            (1 << 1)

#define PM803_LDO_2_VOL_1_8V                       4
#define PM803_LDO_2_VOL_1_9V                       5

#define	QUECTEL_PREBOOT_OPT
#ifdef QUECTEL_PREBOOT_OPT
#define PM807_LDO4_ACTIVE_MODE_REG                 0x7A
#define PM807_LDO4_VSET_3V_BIT                     (0xF)
#define PM807_LDO4_VSET_3V                         (0xD)
#endif

/*#define PM813_REG_FOR_USR_DEF_FLAGS_STR         0xe8 / * pmic813/813s low 4bits * / */
#define PM802_REG_FOR_USR_DEF_FLAGS_STR           0xec /* pmic802/802s/803 high 4bits */
#define UI_FLAG_REBOOT                            0xF

/*
 ***********************************************************
 *                     PID[7:3]|VID[2:0]
 * PM802(GuiLin)       PID = 0x2                     pmic id
 *                     00010    000       PM802_A0   0x10
 *                     00010    001       PM802_A1   0x11
 *                     00010    010       PM802_B0   0x12
 *                     00010    011       PM802_B1   0x13
 *
 * PM813(Ningbo)       PID = 0x6
 *                     00110    000       PM813_A0   0x30
 *                     00110    001       PM813_A1   0x31
 *                     00110    010       PM813_A2   0x32
 *       PM813A3       PID = 0x7
 *                     00111    011       PM813_A3   0x3b // special, PID=7, is diff with other ningbo.
 *
 * PM826(Dalian)       PID = 0x8
 *                     01000    000       PM826_A0   0x40
 *
 * PM812               PID = 0xC
 *                     01100    100       PM812_A0   0x64
 *
 * PM803(GuiLin lite)  PID = 0x3
 *                     00011    000       PM803_A0   0x18
 *
 * PM813S(Ningbo2)     PID = 0x4
 *                     00100    000       PM813S_A0  0x20
 *
 * PM816(Weihai)       PID = 0x5
 *                     00101    000       PM816_A0   0x28
 *
 * PM803L(Guilin Lite2)PID = 0xB
 *                     01011    000       PM803L_A0  0x58
 *
 * PM813L(Ningbo3)     PID = 0x9
 *                     01001    000       PM813L_A0  0x48
 *
 * PM807(Guilin Lite3) PID = 0xD
 *                     01101    000       PM807_A0   0x68
 ***********************************************************
 */

/* PMIC ID:  [7:3] PID,   [2:0] VID */
/*                                                PID */
#define PM812                                     0xC /* PM812: 0x64 */
#define PM813                                     0x6 /* PM813: 0x30(A0), 0x31(A1), 0x32(A2) */
#define PM813A3                                   0x7 /* PM813: 0x3B(A3) */
#define PM802                                     0x2 /* PM802: 0x12(B0), 0x13(B1)... */
#define PM803                                     0x3 /* PM803: 0x18(A0) */
#define PM813S                                    0x4 /* PM813S: 0x20(A0), 0x21(A1) */
#define PM802S                                    0x1 /* PM802S: 0x8(A0), 0x9(A1)*/
#define PM816                                     0x5 /* PM816:  0x28(A0), 0x29(B0), 0x2A(B1) */
#define PM803L                                    0xB /* PM803L: 0x58(A0) */
#define PM813L                                    0x9 /* PM813L: 0x48(A0) */
#define PM807                                     0xD /* PM807: 0x68(A0) */

#define PMIC_UNAVAILABLE                          0xFF

#define PMIC_PID(x)                               (((x) >> 3) & 0x1F) /* bit[7:3] */
#define PMIC_VID(x)                               ((x) & 0x7)         /* bit[2:0] */

#define RESTART_CMD_USB_DOWNLOAD                  'D'
#define RESTART_CMD_NORMAL_POWERON                'N'
#define RESTART_CMD_FORCE_NORMAL_POWERON          'n'
#define RESTART_CMD_PRODUCTION                    'R'

#define RESTART_STATUS_NORMAL_POWERON             'N'
#define RESTART_STATUS_PRODUCTION                 'R'

void pmic_setup(void);
uint8_t pmic_powerup_get_reason(void);
void pmic_fault_wu_en_disable(void);
void pmic_fault_wu_en_enable(void);
void pmic_set_fault_wu(void);
void pmic_set_sw_powerdown(void);
uint8_t pmic_get_id(void);
void pmic_init(void);
int pmic_set_restart_status(char reason);
void pmic_get_restart_cmd(char *r_cmd);
int pmic_set_restart_cmd(char cmd);
int pmic_set_user_flag(char cmd);
void pmic_sw_reboot(void);
void pmic_sw_pdown(void);
void pmic_set_buck1_vol(unsigned int chip_id);
void pmic_set_buck1_skipmode(unsigned int chip_id);
void pmic_print_fault_event_info_for_pm803(void);
void pmic_set_uhs_voltage(void);
void pmic_set_ddr_phy_voltage(void);
void pmic_set_ldo7_en_1_6v_for_craneM_and_pm803(void);
void pmic_set_ldo4_en_3v_for_craneLG_LRH_and_pm807(void);
void pmic_set_fuse_voltage(void);
void pmic_restore_fuse_voltage(void);

#ifdef BOARD_CRANEW_MCU
extern void pmic_LDO_config_for_cranew_mcu_and_pm813s(void);
#endif

#endif
#endif /* __PMIC_H__ */
