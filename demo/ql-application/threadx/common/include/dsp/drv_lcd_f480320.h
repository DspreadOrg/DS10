/**
  ******************************************************************************
  * @file    drv_lcd_f480320.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   LCD 240x320 func file for C header
  * @code    GBK±àÂë
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_LCD_F480320_H__
#define __DRV_LCD_F480320_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
//#define SCREEN_WIDTH                     480
//#define SCREEN_HEIGHT                    320
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief  lcd power on/off
 * @param  on - on/off
 * @retval none
 * @note   none
 */

/**
 * @brief  lcd low-level flush
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_f480320_flush(void);


/**
 * @brief  lcd power on/off
 * @param  on - on/off
 * @retval none
 * @note   none
 */
void lcd_f480320_power( int on );

/**
 * @brief  lcd backlight on/off
 * @param  on - on/off
 * @retval none
 * @note   none
 */
void lcd_f480320_bl_on( int on );


/**
 * @brief  set pixel color in 5-6-5 mode
 * @param  x - x position
 * @param  y - y position
 * @param  color - color in 565 mode
 * @retval 0 is success,otherwise error code
 * @note   none
 */
//void lcd_f480320_set_color565(uint16_t x,uint16_t y,uint16_t color);
void lcd_f480320_set_color565(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t *buff);


/**
 * @brief  set pixel color in 32bit true color mode
 * @param  x - x position
 * @param  y - y position
 * @param  color - color in 32bit mode
 * @retval 0 is success,otherwise error code
 * @note   none
 */
void lcd_f480320_set_color32(uint16_t x,uint16_t y,uint32_t color);


/**
 * @brief  fill lcd with color
 * @param  color - color in 32bit mode
 * @retval none
 * @note   none
 */
void lcd_f480320_fill(uint32_t color);

/**
 * @brief  lcd update request
 * @param  none
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int lcd_f480320_frame_update(void);

/**
 * @brief  lcd 240x320 init
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_f480320_init( void );

#ifdef __cplusplus
}
#endif

#endif /* __DRV_LCD_F480320_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
