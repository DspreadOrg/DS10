/**
  ******************************************************************************
  * @file    disp_ui.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   ui func file for C header
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISP_UI_H__
#define __DISP_UI_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lvgl.h"
#include "disp_res.h"
#include "drv_led_fg00ahk.h"
#include "drv_led_tm1604.h"
#include "drv_tm1721.h"

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    uint8_t type;
    uint8_t datalen;
    uint8_t *data;
} ui_event_t;

typedef struct
{
    uint32_t timeout_ms;
    char text[32];
} ui_tips_data_t;

typedef union
{
    struct custom_msg_pay_t
    {
        int64_t money;
    } pay;

} ui_custom_msg_data_def;

typedef struct
{
    int type;
    uint32_t timeout_ms;
    ui_custom_msg_data_def data;
}ui_custom_msg_t;
/* Exported define -----------------------------------------------------------*/
#define UI_EVNET_SYS_TIPS        0
#define UI_EVNET_MSG             1
#define UI_EVENT_QRCODE          2
#define UI_EVENT_UPDATE          3
#define UI_EVENT_CUSTOM_MSG      4
#define UI_EVENT_CUSTOM_IMG      5

#define UI_MSG_NORMAL            0
#define UI_MSG_PAY_REQUEST       1
#define UI_MSG_PAY_OK            2
#define UI_MSG_REFUND_REQUEST    3
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
 extern lv_obj_t *g_canvas;
 extern lv_obj_t *g_bg;
 extern lv_obj_t *g_bg_blank;
 extern lv_obj_t *g_lv_qr;
 extern lv_obj_t *g_lv_img_qsmzf;
 extern lv_obj_t *g_lv_img_signal;
 extern lv_obj_t *g_lv_label;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  send ui event
 * @param  event_type - event type
 * @param  datalen - event data len
 * @param  data - event data
 * @retval 0 is success,otherwise error code
 * @note   none
 */
 int ui_send_event(uint8_t event_type,uint32_t datalen,uint8_t *data );

/**
 * @brief  send tips event
 * @param  msg - tips msg
 * @param  timeout_ms - how long to show tips
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int ui_send_sys_tips_event( char *msg, uint32_t timeout_ms );

/**
 * @brief  send single line tips event
 * @param  msg - tips msg
 * @param  timeout_ms - how long to show tips
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int ui_send_msg_event(char *msg,uint32_t timeout_ms);

/**
 * @brief  screen display init
 * @param  none
 * @retval 0 is success,otherwise error code
 * @note   none
 */
 int ui_init(void);

void disp_test();
#ifdef __cplusplus
}
#endif

#endif /* __DISP_UI_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
