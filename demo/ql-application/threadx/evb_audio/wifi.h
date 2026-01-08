/**
  ******************************************************************************
  * @file    wifi.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   wifi func file for C header
  * @code    GBK±àÂë
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODULE_WIFI_H__
#define __MODULE_WIFI_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
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

// wifi ap name init
//void WifiConfigApName(void);
extern int WifiCtrlInit(void);

// wifi network config
char WifiConfigSet(void);

#ifdef __cplusplus
}
#endif

#endif /* __FS_MISC_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
