#ifndef __GPIO_H__
#define __GPIO_H__

//#define CS10                   0
//#define CS20                   1
//#define CS21                   2
//#define ID_CS10C              4
//#define ID_CS50C              5
//#define ID_CS56C              6
//#define DEVTYPE_NOT_SUPPORT    7

enum {
    CS10=0,
//    CS20,
//    CS21,
//    ID_S2V,
//    ID_CS10CK,
//    ID_CS50CK,
//    ID_CS56CK,
//	ID_CS56ACK,
//    ID_CS62CK,
//    ID_CS56AC,
//    ID_CS50KC,
//    ID_CS56KC,
//    ID_CS10KC,
//    ID_CS21KC,
//	ID_CS56AAK,
//	ID_CS50AK,
//	ID_CS56AK,
//	ID_CS10AK,
//	ID_CS21AK,
	ID_DS10AK,
	ID_DS10M,
    DEVTYPE_NOT_SUPPORT,
};

//#define GPIO_BUTTON_CMD_LONG_PRESS      0
//#define GPIO_BUTTON_CMD_SHORT_PRESS     1
//#define GPIO_BUTTON_CMD_REPEAT          2
//#define GPIO_BUTTON_CMD_RELEASE         3
//#define GPIO_BUTTON_CMD_DOUBLE_CLICK    4
//#define GPIO_BUTTON_CMD_NULL            5
//#define GPIO_BUTTON_CMD_MAX             6  // do not edit

typedef enum {
    GPIO_BUTTON_CMD_LONG_PRESS=0,
    GPIO_BUTTON_CMD_SHORT_PRESS,
    GPIO_BUTTON_CMD_DOUBLE_CLICK,
    GPIO_BUTTON_CMD_REPEAT,
    GPIO_BUTTON_CMD_RELEASE,
    GPIO_BUTTON_CMD_MAX,
}GPIO_BUTTON_STAT;

#define GPIO_BUTTON_CMD_NULL     GPIO_BUTTON_CMD_MAX

typedef enum {
    GPIO_BUTTON_0,
    GPIO_BUTTON_1,
    GPIO_BUTTON_2,
    GPIO_BUTTON_3,
    GPIO_BUTTON_4,
    GPIO_BUTTON_5,
    GPIO_BUTTON_6,
    GPIO_BUTTON_7,
    GPIO_BUTTON_NUM,
}GPIO_BUTTON_ID;

typedef struct
{
    GPIO_BUTTON_STAT cmd;
    GPIO_BUTTON_ID id;
}GPIO_Button_Cmd_Info;

typedef struct
{
    uint32_t repeat_Press_Hold_Time_Ms;
    uint32_t repeat_Period_Ms;
}Gpio_Button_Repeat;


typedef struct
{
    GPIO_BUTTON_ID button_Id;
    uint32_t button_Value;
    uint32_t short_Press_Hold_Time_Ms;
    uint32_t long_Press_Hold_Time_Ms;
    uint32_t double_click_Time_Ms;
    Gpio_Button_Repeat *repeat_Mode;
//    uint8_t button_cmd_list[GPIO_BUTTON_CMD_MAX];
}Gpio_Button_Info;

typedef struct
{
	GPIO_BUTTON_ID Id;
	uint8_t button_cmd_val[GPIO_BUTTON_CMD_MAX];
}Gpio_Button_Cfg;

//typedef enum {
//
//    SPEAKER_PWREN_INDEX = 0,
//    WIFI_MODULE_INDEX,
//    WIFI_WAKEUP_INDEX,
////    CHARGER_INS_INDEX,
//    CHARGER_STAT_INDEX,
//    FUNCTION_BUTTON_INDEX,
//    VOLUME_BUTTON_INDEX,
//    RED_LED_INDEX,
//    GREEN_LED_INDEX,
//    BLUE_LED_INDEX,
//    BATTERY_CONTROL_INDEX,
////    CHECK_LEVEL_INDEX,
//    ADD_VOLUME_BUTTON_INDEX,
//
//    VOLUME_BUTTON_INDEX_MAX,
//}ePerIndex;

typedef struct {
    unsigned short gpio_num;
    unsigned char pin_pull;
    unsigned char pin_level;
}gpio_pin_t;

typedef struct
{
    unsigned short SDA;
    unsigned short SCK;
    unsigned short CSB;
    unsigned short PWR;
    unsigned short RST;
    unsigned short A0;
    unsigned short BLIGHT;
}SEG_LCD_PORT;

typedef struct
{
    unsigned short pwr_pin;
    unsigned short din_pin;
    unsigned short clk_pin;
}SEG_1604_PORT;

typedef union
{
	SEG_LCD_PORT SEG_LCDPORT;
	SEG_1604_PORT SEG_LEDPORT;
}SEG_PIN_CONFIG_T;

typedef struct
{
    unsigned short CS;
    unsigned short RS;
    unsigned short SDA;
    unsigned short SCL;
    
    unsigned short RST;
    unsigned short PWR;
    unsigned short BLIGHT;
}DOT_LCD_PIN_CONFIG_T;

//#define HAS_SEG_PIN				(1<<0)
//#define HAS_LCD_PIN				(1<<1)
//#define HAS_SEG_LED_PIN		(1<<2)

typedef struct
{
    uint32_t has_lcd_fg7864 :1;
    uint32_t has_led_fg00ahk :1;
    uint32_t has_led_tm1604 :1;
    uint32_t has_lcd_f240320 :1;
}dev_opt_def;

typedef struct
{
  dev_opt_def opt;
  unsigned short G_PWR_PIN;
  SEG_PIN_CONFIG_T SEG_CFG;
  DOT_LCD_PIN_CONFIG_T LCD_CFG;
}DISP_PIN_CONFIG_T;

typedef struct
{
    uint8_t currentset;
    gpio_pin_t speaker;
    gpio_pin_t wifi_power;
    gpio_pin_t wifi_wakeup;
    gpio_pin_t wifi_update;
    gpio_pin_t wifi_en;
//    gpio_pin_t charger_ins;
    gpio_pin_t charger_stat;
    gpio_pin_t function;
    gpio_pin_t volume_down;
    gpio_pin_t volume_up;
    gpio_pin_t led_red;
    gpio_pin_t led_green;
    gpio_pin_t led_blue;
    gpio_pin_t battery_ctrl;
//    gpio_pin_t wifi_pwr_level;
//    SEG_PIN_CONFIG_T SEG_PIN;
//    DOT_LCD_PIN_CONFIG_T LCD_PIN;
    DISP_PIN_CONFIG_T DISP_DEF;
}dev_pin_config_t;

//typedef union
//{
//    dev_opt_def bits;
//    uint32_t value;
//} dev_opt_t;

typedef struct{
    uint8_t id;
    const char *devname;
//    const char *module;
    const dev_pin_config_t *pins;
    const Gpio_Button_Info *btconf;
//    const dev_opt_t *opt;
    void * AudioMediaVETable;
}dev_config_t;

//extern int ym_switch_peripheral_function(ePerIndex num, uint8_t OnFlag);
//extern int ym_get_peripheral_function_state(ePerIndex num);
//extern int ym_get_peripheral_function_gpio(ePerIndex num);
extern int ym_get_config_current_device_type(void);
extern const char *ym_get_current_device_name(void);

extern int getAddVolumeButtonGpio(void);
extern void gpio_speaker_onoff(uint8_t status);

extern void gpio_wifi_power_onoff(uint8_t OnFlag);
extern void gpio_wifi_wakeup_Set(uint8_t OnFlag);

extern void gpio_battery_control_pin_onoff(uint8_t OnFlag);

extern void setRedLedOnOff(uint8_t OnFlag);
extern void setBlueLedOnOff(uint8_t OnFlag);
extern void setGreenLedOnOff(uint8_t OnFlag);

extern int getFunctionButtonGpio(void);
extern int getVolumeButtonGpio(void);

extern int get_device_type(void);
const char *get_device_name(void);
const dev_config_t *get_device_config(void);

//extern int get_charger_ins_gpio(void);
extern gpio_pin_t get_charger_stat_gpio(void);
extern int get_charger_control_gpio(void);

void load_device_config(const char *devname);

#endif
