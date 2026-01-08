/**
  ******************************************************************************
  * @file    drv_lcd_fg7864.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD 78x64 func file for C header
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_TM1721_H__
#define __DRV_TM1721_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ql_rtos.h"
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
#define KB_NUM_CNT          10
#define KB_POINT_CNT        9
typedef struct
{
    uint8_t num[KB_NUM_CNT];
    uint8_t point[KB_POINT_CNT];
    uint8_t gprs;
    uint8_t wifi;
    uint8_t yuan;

    uint8_t level_1;
    uint8_t level_2;
    uint8_t level_3;
    uint8_t level_4;
}kb_seg_info_t;

#define KB_EVENT_SCAN      0x01

typedef struct
{
    uint8_t type;
    uint8_t datalen;
    uint8_t *data;
} kb_event_t;

typedef void (*kb_input_acllback_t)( int , void*);

/* Exported define -----------------------------------------------------------*/
#define KB_INPUT_OK        1
#define KB_INPUT_ERR       2
#define KB_INPUT_SET       3
#define KB_INPUT_REPEAT    4
#define KB_INPUT_RETURN    5
#define KB_INPUT_PAY       6
#define KB_INPUT_CLEAR     7
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


/**************************************************************
 *                      Hight level function
 ***************************************************************/
/**
 * @brief  设置输入回调函数，键盘按键时触发
 * @param  cb - callback handler
 * @retval none
 * @note   none
 */
void keyboard_set_input_callback(kb_input_acllback_t cb);

/**
 * @brief  键盘清屏
 * @param  none
 * @retval none
 * @note   none
 */
void keyboard_clear_show(void);

/**
 * @brief  键盘全显
 * @param  none
 * @retval none
 * @note   none
 */
void keyboard_fill_show(void);

/**
 * @brief  键盘默认显示，显示0元
 * @param  none
 * @retval none
 * @note   none
 */
void keyboard_idle_show(void);

/**
 * @brief  键盘显示开关
 * @param  on - on(1),off(0)
 * @retval none
 * @note   none
 */
void keyboard_onoff_show(int on);

/**
 * @brief  设置键盘显示的信号强度
 * @param  level - 信号级别，0-4
 * @retval none
 * @note   none
 */
void keyboard_set_signal(uint8_t level);

/**
 * @brief  设置键盘显示的模式
 * @param  mode - 模式，0-无，1-gprs，2-wifi
 * @retval none
 * @note   none
 */
void keyboard_set_mode(uint8_t mode);

/**
 * @brief  初始化键盘硬件和显示
 * @param  none
 * @retval none
 * @note   none
 */
void keyboard_init( void );

#ifdef __cplusplus
}
#endif

#endif /* __DRV_TM1721_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
