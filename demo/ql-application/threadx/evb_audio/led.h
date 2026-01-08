#ifndef __LED_H__
#define __LED_H__
#include "prj_common.h"
#define PRJCONF_LED_BLINK_PERIOD 500
#define PRJCONF_LED_FAST_BLINK_PERIOD 300
#define PRJCONF_LED_FLOW_BLINK_PERIOD 1000

typedef enum
{
	BLACK_COLOR=0,     // ∫⁄
	RED_COLOR,       // ∫Ï
	GREEN_COLOR,     // ¬Ã
	BLUE_COLOR,       // ¿∂
	WHITE_COLOR,
	NORMAL_COLOR,
	RGB_BLINK,        //∫Ï¬Ã¿∂ΩªÃÊ…¡À∏
	RB_BLINK,		  //∫Ï¿∂ΩªÃÊ…¡À∏
	RGB_COLOR,
}led_color;

typedef enum
{
    TERM_NORMAL,
    TERM_ABNORMAL,
    TERM_ABNORMAL_EXT,
	TERM_INIT_START,
	TERM_INIT_END,
	TERM_POWEROFF,
	TERM_OTA_START,
	TERM_OTA_FAIL,
	TERM_OTA_OK,
//	TERM_WAIT,
//	TERM_BUSY,
	TERM_NET_START,
	TERM_NET_CON,
	TERM_NET_DIS,
	TERM_NET_FAILED,
	TERM_NET_APMODE,
	TERM_NET_ABNORMAL,
	TERM_LTE_NET_ABNORMAL,
	TERM_FAULT,
	TERM_REG_FAIL,
	TERM_PARAM_ERR,
	TERM_CHARGE_START,
	TERM_CHARGE_END,
	TERM_CHARGE_FULL,
	TERM_LOWBAT,
	TERM_NO_SIM,
}LedModeDef;


extern void ChargeStatLedSet(int OnOff);
//extern void gpio_led_pin_init(void);
//extern void gpio_led_pin_deinit(void);
//extern void setRedLedOnOff(int on);
//extern void setBlueLedOnOff(int on);
//extern void setGreenLedOnOff(int on);
extern void color_led_func(led_color led);
extern void platform_led_timer_start(led_color led, uint32_t periodic, led_color led_type);
extern void platform_led_timer_stop(void);
extern void gpio_led_task_init(void);
extern void TermLedShow(LedModeDef mode);

#define GREEN_LED_SET 	(1<<0)
#define BLUE_LED_SET 		(1<<1)
#define RED_LED_SET 		(1<<2)

typedef enum
{
	FlashFast,
	FlashSlow,
	AlwaysOn,
	AlwaysOff,
	Blink,
	Breath,
}LedStatSet;

typedef enum  {
	GPIO_TYPE = 0,
	PWM_TYPE = 1,
}LedTypeSet;


typedef struct {
	unsigned char LedSet;
	unsigned int SetTimeMs;
} LedStatSeq;

//void led_onoff_test(void);

//extern void LedModeSet(int LedSel,LedStatSet Mode);
//extern void led_init(void);
//extern void setBreathLedMode(LedTypeSet type);
//extern void setBreathLedTime(int time);//µ•Œªms

//extern void setBreathLedMode(LedTypeSet type);
//extern void setBreathLedTime(int time);//µ•Œªms 
//extern void Breath_led_Deinit(void);

#endif
