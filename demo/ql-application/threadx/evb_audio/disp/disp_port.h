/**
  ******************************************************************************
  * @file    disp_port.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   display port func file for C header
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_PORT_H__
#define __DISP_PORT_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "drv_led_fg00ahk.h"
#include "drv_led_tm1604.h"
#include "disp_ui.h"
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
// CS10/20/21或不带屏的设备建议关闭相应的显示屏支持，可以减小固件大小
#define DEV_FG00AHK_0964	2
#define DEV_FG00AHK_1921	3
#define DEV_FG00AHK_1921B	4
#define DEV_FG7864_SUPPORT      0
#define DEV_FG00AHK_SUPPORT     1
#define DEV_TM1604_SUPPORT      0
#define DEV_KEYBOARD_SUPPORT    0
#define DEV_F240320_SUPPORT     0
#define DEV_F480320_SUPPORT     1

#if DEV_FG00AHK_SUPPORT
//CS50采用硬件配置自动识别，请屏蔽以下型号指定
#define DEV_FG00AHK_TYPE	DEV_FG00AHK_1921B
#endif

#if ((DEV_FG7864_SUPPORT + DEV_F240320_SUPPORT + DEV_F480320_SUPPORT)>1)
#error "LCD78X64 or LCD240X320 or LCD480X320 can not set at the same time! "
#endif

#if DEV_FG7864_SUPPORT
#include "drv_lcd_fg7864.h"
#define SCREEN_BG_COLOR        lv_color_make(255,255,255)
#define SCREEN_FT_COLOR        lv_color_make(0,0,0)
#define QR_CODE_SIZE              64
#elif DEV_F240320_SUPPORT
#include "drv_lcd_f240320.h"
#define SCREEN_BG_COLOR        lv_color_make(255,255,255)
#define SCREEN_FT_COLOR        lv_color_make(0,0,0)
#define QR_CODE_SIZE              192
#elif DEV_F480320_SUPPORT
#include "drv_lcd_f480320.h"
#define SCREEN_BG_COLOR        lv_color_make(255,255,255)
#define SCREEN_FT_COLOR        lv_color_make(0,0,0)
#define QR_CODE_SIZE              192
#else
#include "drv_lcd_fg7864.h"
#define SCREEN_BG_COLOR        lv_color_make(255,255,255)
#define SCREEN_FT_COLOR        lv_color_make(0,0,0)
#define QR_CODE_SIZE              64
#endif


// up/down icon show
#define DISP_SHOW_UP         1
#define DISP_SHOW_DOWN       2
#define DISP_SHOW_UPDOWN     3
#define DISP_HIDE_UPDOWN     0

// default hold on time
#define DISP_HOLDON_MS       (20 * 1000)
#define DISP_HOLDON_FOREVER  0xffffffff
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  disp port init
 * @param  none
 * @retval none
 * @note   none
 */
void disp_port_init( void );

/**
 * @brief  disp high level ui init
 * @param  none
 * @retval none
 * @note   none
 */
void disp_ui_init( void );

/**
 * @brief  disp power off
 * @param  none
 * @retval none
 * @note   none
 */
void disp_poweroff_msg(void);

/**
 * @brief  switch none mode
 * @param  none
 * @retval none
 * @note   none
 */
void disp_none_mode(void);

/**
 * @brief  switch gprs mode
 * @param  none
 * @retval none
 * @note   none
 */
void disp_gprs_mode(void);

/**
 * @brief  set gprs state
 * @param  is_err - error or ok
 * @retval none
 * @note   none
 */
void disp_set_gprs_error(uint8_t is_err);

/**
 * @brief  set gprs signal state
 * @param  level - sigal level,0-4
 * @retval none
 * @note   none
 */
void disp_set_gprs_signal_level(uint8_t level);

/**
 * @brief  set wifi signal state
 * @param  level - sigal level,0-4
 * @retval none
 * @note   none
 */
void disp_set_wifi_signal_level(uint8_t level);

/**
 * @brief  switch wifi mode
 * @param  none
 * @retval none
 * @note   none
 */
void disp_wifi_mode(void);

/**
 * @brief  set wifi state
 * @param  is_err - error or ok
 * @retval none
 * @note   none
 */
void disp_set_wifi_error(uint8_t is_err);

/**
 * @brief  set up/down state
 * @param  state - up/down flag
 * @retval none
 * @note   none
 */
void disp_set_updown_state(uint8_t state);

/**
 * @brief  update vol icon
 * @param  is_show_vol - is show vol str
 * @retval none
 * @note   none
 */
void disp_vol_update( uint8_t is_show_vol );

/**
 * @brief  set backlight level
 * @param  level - level 0-100
 * @retval none
 * @note   none
 */
void disp_set_backlight(uint8_t level );

/**
 * @brief  set bat level state
 * @param  level - bat level
 * @retval none
 * @note   none
 */
void disp_set_bat_level(uint8_t level);

/**
 * @brief  disp set ota state
 * @param  percent - level 0-100
 * @retval none
 * @note   none
 */
void disp_set_ota_state(int percent);

/**
 * @brief  disp network connecting
 * @param  none
 * @retval none
 * @note   none
 */
void disp_net_connecting(void);

/**
 * @brief  disp service connecting
 * @param  none
 * @retval none
 * @note   none
 */
void disp_service_connecting(void);

/**
 * @brief  disp set network connect state
 * @param  is_err - error or ok
 * @retval none
 * @note   none
 */
void disp_set_net_connect_error(uint8_t is_err);

/**
 * @brief  disp set server connect state
 * @param  is_err - error or ok
 * @retval none
 * @note   none
 */
void disp_set_service_connect_error(uint8_t is_err);

/**
 * @brief  disp param config state
 * @param  is_err - error or ok
 * @retval none
 * @note   none
 */
void disp_set_param_config_error(uint8_t is_err);


/**
 * @brief  disp charge state
 * @param  is_charge - is charge state
 * @retval none
 * @note   none
 */
void disp_set_charge(uint8_t is_charge);

/**
 * @brief  disp money
 * @param  money - recv pay
 * @param  ts - timestamp
 * @param  flags - flags
 * @retval none
 * @note   none
 */
void disp_set_paymsg(int64_t money,uint32_t ts,uint32_t flags);

/**
 * @brief  show tips on main area
 * @param  msg - tips msg
 * @param  timeout_ms - how long msg show
 * @retval none
 * @note   none
 */
void disp_set_msg(char *msg,uint32_t timeout_ms);

/**
 * @brief  show tips on slave area
 * @param  msg - tips msg
 * @param  timeout_ms - how long tips show
 * @retval none
 * @note   none
 */
void disp_set_tips(char *msg,uint32_t timeout_ms);

/**
 * @brief  show custom msg
 * @param  data - custom data
 * @param  datalen - data len
 * @retval none
 * @note   none
 */
void disp_set_custom_msg(void *data,int datalen);

/**
 * @brief  set msg show on screen,only for led-type device
 * @param  msg - tips msg
 * @param  timeout_ms - how long msg show
 * @retval none
 * @note   none
 */
void disp_set_led_msg(char *msg,uint32_t timeout_ms);

/**
 * @brief  disp update qrcode
 * @param  qr_content - content to make qrcode
 * @param  content_len - content len
 * @retval none
 * @note   none
 */
void disp_set_qrcode(char *qr_content,int content_len);

/**
 * @brief  disp update request
 * @param  none
 * @retval none
 * @note   none
 */
void disp_update_request(void);


/**
 * @brief  set disp to on and hold on timeout_ms
 * @param  on - on or off
 * @param  holdon_ms - time to hold on
 * @retval error code
 * @note   none
 */
void disp_onoff_request(uint8_t on,uint32_t holdon_ms);

/**
 * @brief  set sleep enable or not
 * @param  enable - enable or disable
 * @retval error code
 * @note   none
 */
void disp_sleep_enable(uint8_t enable);

/**
 * @brief  start flash
 * @param  mode - flash mode
 * @param  repeat - repeat times
 * @param  period_ms - flash period in ms
 * @retval error code
 * @note   none
 */
void disp_show_flash(uint8_t mode, uint32_t repeat,uint32_t period_ms);

/**
 * @brief  show record index
 * @param  index - record index
 * @param  timeout_ms - how long msg show
 * @retval error code
 * @note   none
 */
void disp_record_index(int index,uint32_t timeout_ms);

/**
 * @brief  set clock enable or not
 * @param  enable - enable or disable
 * @retval error code
 * @note   none
 */
void disp_clock_enable(uint8_t enable);

/**
 * @brief  show busy state
 * @param  none
 * @retval error code
 * @note   none
 */
void disp_state_busy(void);

/**
 * @brief  show idle state
 * @param  none
 * @retval error code
 * @note   none
 */
void disp_state_idle(void);

/**
 * @brief  factory test,set screen to all on/off
 * @param  on - on/off state
 * @retval error code
 * @note   none
 */
void disp_factorytest_onoff(int on);

void disp_set_img();

#ifdef __cplusplus
}
#endif

#endif /* __DISP_PORT_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
