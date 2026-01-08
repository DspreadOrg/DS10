/**
  ******************************************************************************
  * @file    drv_lcd_fg7864.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD 78x64 func file for C header
  * @code    GBK±àÂë£¬
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DRV_LCD_FG7864_H__
#define __DRV_LCD_FG7864_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ql_rtos.h"
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    uint32_t bat_level_4 :1;
    uint32_t bat_level_3 :1;
    uint32_t bat_level_2 :1;
    uint32_t bat_level_1 :1;
    uint32_t bat_outline :1;
    uint32_t bat_charge :1;
    uint32_t vol_level_3 :1;
    uint32_t vol_level_2 :1;
    uint32_t vol_level_1 :1;
    uint32_t vol_outline :1;
    uint32_t donw :1;
    uint32_t up :1;
    uint32_t wifi_error :1;
    uint32_t wifi_outline :1;
    uint32_t gprs_underline :1;
    uint32_t gprs_level_4 :1;
    uint32_t gprs_level_3 :1;
    uint32_t gprs_level_2 :1;
    uint32_t gprs_level_1 :1;
    uint32_t gprs_error :1;
    uint32_t gprs_2g :1;
    uint32_t gprs_4g :1;
} lcd_icon_t;

#define FRAME_FLAG_ICON                  0x01
#define FRAME_FLAG_PIXLE                 0x02
#define FRAME_FLAG_FLUSH                 0X04
#define FRAME_FALG_ALL                   0xffffffff

/* Exported define -----------------------------------------------------------*/
// ROTATE
#define DISP_ROTATE_0                    0
#define DISP_ROTATE_90                   1
#define DISP_ROTATE_180                  2
#define DISP_ROTATE_270                  3

// lcd real WIDTH & HEIGHT
//#define SCREEN_PHY_WIDTH                 78
//#define SCREEN_PHY_HEIGHT                64

// virtual WIDTH & HEIGHT for display driver
//#define SCREEN_WIDTH                     64
//#define SCREEN_HEIGHT                    78
#define ROTATE_MODE                      DISP_ROTATE_90

// icon define
#define ICON_BAT_LEVEL_4                 0
#define ICON_BAT_LEVEL_3                 1
#define ICON_BAT_LEVEL_2                 2
#define ICON_BAT_LEVEL_1                 3
#define ICON_BAT_OUTLINE                 4
#define ICON_BAT_CHARGE                  5
#define ICON_VOL_LEVEL_3                 6
#define ICON_VOL_LEVEL_2                 7
#define ICON_VOL_LEVEL_1                 8
#define ICON_VOL_OUTLINE                 9
#define ICON_DOWN                        10
#define ICON_UP                          11
#define ICON_WIFI_ERROR                  12
#define ICON_WIFI_OUTLINE                13
#define ICON_GPRS_UNDERLINE              14
#define ICON_GPRS_LEVEL_4                15
#define ICON_GPRS_LEVEL_3                16
#define ICON_GPRS_LEVEL_2                17
#define ICON_GPRS_LEVEL_1                18
#define ICON_GPRS_ERROR                  19
#define ICON_GPRS_2G                     20
#define ICON_GPRS_4G                     21

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern ql_mutex_t lcd_flush_lock;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

// spi clk and sda test
 void spi_test(void);

/**
 * @brief  flush lcd show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_flush( );

/**
 * @brief  set/clr pixel on (x,y)
 * @param  x - x positon
 * @param  y - y position
 * @param  level - level,0 is off
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int lcd_set_xy( int x, int y, uint8_t level );

/**
 * @brief  clear lcd vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_clear( );

/**
 * @brief  clear lcd vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_clear_show( );

/**
 * @brief  fill lcd vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_fill();

/**
 * @brief  fill lcd vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_fill_show();


/**
 * @brief  soft control lcd on/off
 * @param  on - on/off,0 is off
 * @retval none
 * @note   none
 */
void lcd_onoff(int on);


/**
 * @brief  set lcd backlight level
 * @param  level - brightness level,0-100
 * @retval none
 * @note   none
 */
void lcd_set_bl_level(uint8_t level);


/**
 * @brief  lcd hal init
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_init( void );

/**
 * @brief  flush icon show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_icon_flush(void);

/**
 * @brief  clear icon vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_icon_clear(void);

/**
 * @brief  clear icon vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_icon_clear_show(void);

/**
 * @brief  fill icon vram but not update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_icon_fill(void);

/**
 * @brief  fill icon vram and update show
 * @param  none
 * @retval none
 * @note   none
 */
void lcd_icon_fill_show(void);

/**
 * @brief  set/clr icon
 * @param  icon - icon index
 * @param  level - level,0 is off
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int lcd_set_icon(uint8_t icon,uint8_t level);

/**
 * @brief  request lcd update
 * @param  frame_mask - update flag mask
 * @retval 0 is success,otherwise error code
 * @note   none
 */
int lcd_frame_update(uint8_t frame_mask);


void lcd_f7864_set_mono(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t *buff);
int lcd_f7864_frame_update(void);

/**************************************************************
 *                      Hight level function
 ***************************************************************/
/**
 * @brief  set gprs icon by level
 * @param  level - signal level
 * @retval none
 * @note   none
 */
void lcd_set_gprs_level(uint8_t level);

/**
 * @brief  set vol icon by level
 * @param  level - vol level
 * @retval none
 * @note   none
 */
void lcd_set_vol_level(uint8_t level);

/**
 * @brief  set bat icon by level
 * @param  level - bat level
 * @retval none
 * @note   none
 */
void lcd_set_bat_level(uint8_t level);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_LCD_FG7864_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
