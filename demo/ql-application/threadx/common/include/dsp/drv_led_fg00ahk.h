/**
  ******************************************************************************
  * @file    led_fg00ahk.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LED FG00AHK func file for C header
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_FG00AHK_H__
#define __LED_FG00AHK_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    uint32_t is_m_busy :1;
    uint32_t is_s_busy :1;
    uint32_t is_on :1;            // on or off
    uint32_t en_sleep :1;
    uint32_t m_mode :4;
    uint32_t en_clock :1;         // enable clock
    uint32_t show_clock :1;       // clock time is showing or not(read only)
    uint32_t is_visible :1;       // visible or hidden
    uint32_t need_update :1;
} led_fg00_status_t;

#define LED_M_NUM_COUNT           8
#define LED_S_NUM_COUNT           6
#define LED_E_NUM_COUNT           2
#define LED_M_POINT_COUNT         8
#define LED_M_SPLIT_COUNT         2
#define LED_S_SPLIT_COUNT         2
typedef struct
{
    uint8_t m_num[LED_M_NUM_COUNT];    // 主屏大数字(M)
    uint8_t s_num[LED_S_NUM_COUNT];    // 主屏小数字（S,时钟）
    uint8_t e_num[LED_E_NUM_COUNT];    // 主屏小数字（E,元和电量之间）
    uint8_t g_num;                     // 主屏小数字(G)，GPRS数字
    uint8_t m_point[LED_M_POINT_COUNT];  // 主屏大数字的小数点
    uint8_t m_split[LED_M_SPLIT_COUNT];  // 时钟的冒号“：”
    uint8_t s_split[LED_S_SPLIT_COUNT];  // 副屏时钟的冒号“：”
    uint8_t yuan;

    uint8_t gprs_g;
    uint8_t gprs_err;
    uint8_t gprs_level_1;
    uint8_t gprs_level_2;
    uint8_t gprs_level_3;
    uint8_t gprs_level_4;

    uint8_t wifi_err;
    uint8_t wifi_level_1;
    uint8_t wifi_level_2;
    uint8_t wifi_level_3;
    uint8_t wifi_level_4;

    uint8_t speaker;
    uint8_t speaker_level_1;
    uint8_t speaker_level_2;
    uint8_t speaker_level_3;
    uint8_t speaker_level_4;
    uint8_t speaker_level_5;

    uint8_t bat;
    uint8_t bat_charge;
    uint8_t bat_level_1;
    uint8_t bat_level_2;
    uint8_t bat_level_3;
    uint8_t bat_level_4;

    uint8_t bt;
    uint8_t bt_err;
    uint8_t bt_conn; 

    led_fg00_status_t status;
}led_fg00_info_t;

typedef struct
{
    uint8_t type;
    uint8_t datalen;
    uint8_t *data;
}led_fg00_event_t;

typedef struct
{
    uint32_t timeout_ms;
    char text[32];
} fg00_tips_data_t;

typedef struct
{
    uint32_t timeout_ms;
    char text[3];
} fg00_emsg_data_t;
/* Exported define -----------------------------------------------------------*/
// 主屏模式
#define FG00_M_MODE_NONE         0
#define FG00_M_MODE_STR          1  // 显示普通字符
#define FG00_M_MODE_NET_CONN     2  // 显示网络连接中
#define FG00_M_MODE_SVC_CONN     3  // 显示服务连接中
#define FG00_M_MODE_CLOCK        4  // 显示时钟
#define FG00_M_MODE_BUSY         5  // 显示忙碌中

// 闪烁模式
#define FG00_FLASH_ALL           0
#define FG00_FLASH_4G            1
#define FG00_FLASH_VOL           2
#define FG00_FLASH_CTRL_MAX      3

// led events
#define FG00_EVENT_UPDATE        0
#define FG00_EVENT_MSTR          1
#define FG00_EVENT_SSTR          2
#define FG00_EVNNT_WAKEUP        3
#define FG00_EVENT_OFF           4
#define FG00_EVENT_NET_CONN      5
#define FG00_EVENT_SVC_CONN      6
#define FG00_EVENT_FLASH         7
#define FG00_EVENT_ESTR          8
#define FG00_EVENT_BUSY          9
#define FG00_EVENT_PAYSTR        10
#define FG00_EVENT_FLUSH         11

// led显示的字符
#define FG00_CHAR_BLANK          36
#define FG00_CHAR_SPLIT          37


#define LED_FG00_BLANK_STR       "      "
#define LED_FG00_IDLE_STR        "   0.00"
#define LED_FG00_OFF_STR         " 0FF "

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern led_fg00_info_t g_fg00_info;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  flush lcd show
 * @param  none
 * @retval none
 * @note   none
 */
 void led_fg00_flush();

/**
 * @brief  set/clr pixel on (x,y)
 * @param  x - x positon
 * @param  y - y position
 * @param  level - level,0 is off
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int led_fg00_set_com_seg(int com,int seg,uint8_t level);

/**
 * @brief  clear lcd vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void led_fg00_clear();

/**
 * @brief  clear lcd vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void led_fg00_clear_show();

/**
 * @brief  fill lcd vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void led_fg00_fill();

/**
 * @brief  fill lcd vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void led_fg00_fill_show();


/**
 * @brief  soft control lcd on/off
 * @param  on - on/off,0 is off
 * @retval none
 * @note   none
 */
void led_fg00_onoff(int on);


/**
 * @brief  set lcd backlight level
 * @param  level - brightness level,0-100
 * @retval none
 * @note   none
 */
void led_fg00_set_bl_level(uint8_t level);

/**************************************************************
 *                      Hight level function
 ***************************************************************/
/**
 * @brief  set wifi icon by level
 * @param  level - signal level
 * @retval none
 * @note   none
 */
void fg00_set_wifi_level(uint8_t level);

/**
 * @brief  set gprs icon by level
 * @param  level - signal level
 * @retval none
 * @note   none
 */
void fg00_set_gprs_level(uint8_t level);

/**
 * @brief  set bat icon by level
 * @param  level - bat level
 * @retval none
 * @note   none
 */
void fg00_set_bat_level(uint8_t level);

/**
 * @brief  set vol icon by level
 * @param  level - vol level
 * @retval none
 * @note   none
 */
void fg00_set_vol_level(uint8_t level);

/**************************************************************
 *                      LED thread
 ***************************************************************/
/**
 * @brief  show string on led
 * @param  event_type - event type
 * @param  msg - string to show
 * @param  timeout_ms - how long to show
 * @retval error code
 * @note   none
 */
int led_fg00_show_string(uint8_t event_type,char *msg,uint32_t timeout_ms);

/**
 * @brief  show digit on led
 * @param  event_type - event type
 * @param  value - string to show
 * @param  timeout_ms - how long to show
 * @retval error code
 * @note   none
 */
int led_fg00_show_digit(uint8_t event_type,int64_t value,uint32_t timeout_ms);

/**
 * @brief  show emsg on led
 * @param  strmsg - string msg(first use)
 * @param  num - digit,use when strmsg is null
 * @param  timeout_ms - how long to show
 * @retval error code
 * @note   none
 */
int led_fg00_show_emsg(char *strmsg,int num, uint32_t timeout_ms);

/**
 * @brief  set led to on and hold on timeout_ms
 * @param  timeout_ms - time to hold on
 * @retval error code
 * @note   none
 */
int led_fg00_wakeup(uint32_t timeout_ms);

/**
 * @brief  turn off led show
 * @param  none
 * @retval error code
 * @note   none
 */
int led_fg00_off(void);

/**
 * @brief  set sleep enable or not
 * @param  enable - enable or disable
 * @retval error code
 * @note   none
 */
void led_fg00_sleep_enable(uint8_t enable);

/**
 * @brief  led flash
 * @param  mode - flash mode
 * @param  repeat - repeat times
 * @param  period_ms - flash period in ms
 * @retval error code
 * @note   none
 */
int led_fg00_flash(uint8_t mode, uint32_t repeat,uint32_t period_ms);

/**
 * @brief  led lightup
 * @param  mode - flash mode
 * @param  showtime_ms - how long to lightup
 * @retval error code
 * @note   none
 */
int led_fg00_lightup(uint8_t mode,uint32_t showtime_ms);

/**
 * @brief  lcd hal init
 * @param  none
 * @retval none
 * @note   none
 */
int led_fg00_init( void );

/**
 * @brief  send ui event
 * @param  event_type - event type
 * @param  datalen - event data len
 * @param  data - event data
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int fg00_send_event(uint8_t event_type,uint32_t datalen,uint8_t *data );


#ifdef __cplusplus
}
#endif

#endif /* __LED_FG00AHK_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
