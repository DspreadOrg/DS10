/**
  ******************************************************************************
  * @file    animate.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD animate func file for C header
  * @code    GBK±àÂë£¬
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_ANIMATE_H__
#define __LCD_ANIMATE_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "disp_ui.h"
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief  charging animate
 * @param  none
 * @retval next trigger in ms
 * @note   none
 */
int animate_battery( void );

/**
 * @brief  charging animate
 * @param  none
 * @retval next trigger in ms
 * @note   none
 */
int animate_signal(void);

/**
 * @brief  update qrcode
 * @param  none
 * @retval next trigger in ms
 * @note   none
 */
int animate_qrcode(void);

/**
 * @brief  run animate task
 * @param  none
 * @retval none
 * @note   none
 */
void animate_task_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __LCD_ANIMATE_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
