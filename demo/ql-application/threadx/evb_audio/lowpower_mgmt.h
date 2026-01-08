/**
  ******************************************************************************
  * @file    lowpower_mgmt.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   µÍ¹¦ºÄ¹¦ÄÜ func file for C header
  * @code    GBK±àÂë£¬
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOWPOWER_MGMT_H__
#define __LOWPOWER_MGMT_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    uint32_t button :1;
    uint32_t mqtt :1;
    uint32_t tts :1;
    uint32_t init :1;
    uint32_t wifi :1;
    uint32_t network :1;
} lpm_bit_def;

typedef union
{
    lpm_bit_def bits;
    uint32_t value;
} lpm_lock_t;
/* Exported define -----------------------------------------------------------*/
#define LPM_LOCK_INIT              0
#define LPM_LOCK_BUTTON            1
#define LPM_LOCK_NETWORK           2
#define LPM_LOCK_TTS               3
#define LPM_LOCK_MQTT              4
#define LPM_LOCK_WIFI              5

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  set lowpower flag
 * @param  lock_type - lock type
 * @param  value - 1 lock the lpm,can not enter lowpwer mode,0 release the lock
 * @retval none
 * @note   none
 */
void lpm_set(uint8_t lock_type,uint8_t value);

/**
 * @brief  lowpower init
 * @param  none
 * @retval none
 * @note   none
 */
void lpm_init(void);



#ifdef __cplusplus
}
#endif

#endif /* __LOWPOWER_MGMT_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
