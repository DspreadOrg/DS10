#ifndef __TM1640_H__
#define __TM1640_H__

#define ADDR_START_TM1640  0xC0
#define ADDR_END_TM1640    0xCF

#define DISABLE_TM1640     0
#define ENABLE_TM1640      1

/*
    0x40  address auto add +1
    0x44  address fixed
*/
 
//  set data cmd , address auto +1
#define SET_DATA_CMD       0x40 

/*
    0x88 | 0x89 | 0x8a | 0x8b  | 0x8c  | 0x8d  | 0x8e  | 0x8f
    1/16 | 2/16 | 4/16 | 10/16 | 11/16 | 12/16 | 13/16 | 14/16
*/

//  display ctrl cmd, pulse width 14/16
#define SET_DISPLAY_CMD    0x8b

//  close display cmd,
#define DIS_DISPLAY_CMD    0x80

#define DELAY_SHORT        0 //80//10//2000//150
#define DELAY_NOP          80//10
#define DELAY_TIM          150

#define NUM_LED_MAX        6

#define FAC_PWR_SHOW       "FAC   " 
#define PWR_ON_SHOW        "8.8.8.8.8.8." 
#define PWR_OFF_SHOW       "0FF.   "
#define REPLAY_RECORD_SHOW "REC.   "

typedef enum {
    LOW = 0,
    HIGH,
}TM1640_Level_e;

/**
  * @brief cmd type.
  */
typedef enum {

    DISPLAY_NUMBER = 1,
    DISPLAY_STRING,
    
    CONNECT_NET,
    CONNECT_NET_SUCCES,
    CONNECT_NET_FAIL,
    
    DISTRI_NET,
    DISTRI_NET_SUCCES,
    DISTRI_NET_FAIL,
    
    DEVICE_OTA,
    
    DIS_PWR_OFF,
    
    REPLAY_RECORD,
    
}LcdCmdType;

/**
  * @brief show number.
  */
typedef enum {
    ONCE = 1,
    LOOP,
}LcdShowNum;

typedef struct {
    uint8_t currentMod;
    uint8_t loopMod;
    uint32_t tim;
    uint16_t pDelTim;
    uint16_t uDelTim;
    int (*pfunc)(void);
    int (*ufunc)(void);
} Tm_Screen_Off_t;
/**
  * @brief Off or On display.
  */
typedef struct led_queue_s{
    LcdCmdType type;
    uint32_t val;
    char str[16];
}LcdQueueS;

void tm1640_init(void);
void display_string(uint8_t *p_dat);
int tm_show_string(char *cszBuf);
void lcd_task_init(void);
int release_lcd_queue(LcdCmdType type, uint32_t val, char *src);
int volume_num_to_show(uint16_t dat);
void factory_pwron_show(void);

#endif


