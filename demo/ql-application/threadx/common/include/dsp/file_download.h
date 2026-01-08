/**
  ******************************************************************************
  * @file    file_download.h
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   文件下载功能 func file for C header
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FILE_DOWNLOAD_H__
#define __FILE_DOWNLOAD_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef void(*http_download_cb_t)(int);
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  set download callback
 * @param  cb - callback(result),fail(result<0),done(result==0),datasize(result>0)
 * @retval none
 * @note   none
 */
void set_http_download_cb(http_download_cb_t cb);

 /**
  * @brief  download file from url to local file system
  * @param  url - download url
  * @param  fpath - local file path
  * @param  fmaxsize - max download size
  * @retval 0 is success,otherwise error code
  * @note   none
  */
 int http_download(char *url,char *fpath,int fmaxsize);



#ifdef __cplusplus
}
#endif

#endif /* __FILE_DOWNLOAD_H__ */

/************************ (C) COPYRIGHT YMZN *****END OF FILE****/
