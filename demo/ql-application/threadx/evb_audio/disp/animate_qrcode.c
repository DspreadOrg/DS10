/**
  ******************************************************************************
  * @file    animate_charge.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD 78x64 func file for C code
  * @code    GBK���룬
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_spi.h"
#include "ql_pwm.h"
#include "ql_gpio.h"
#include "ql_nw.h"
#include "ql_power.h"
#include "prj_common.h"
#include "systemparam.h"
#include "public_api_interface.h"
#include "lvgl.h"
#include "res.h"
#include "disp_port.h"
#include "terminfodef.h"
#include "animate.h"

#define LOG_INFO(...)           do{printf("[ANM]: "); printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define QR_CONTENT              "Welcome to use the cloud speaker"
#define QR_CONTENT2             "Welcome to use the cloud speaker"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

// update qrcode
int animate_qrcode(void)
{
    static uint32_t timer_next = 0;
    static uint8_t qr_index = 0;
    uint32_t tick_ms = ql_rtos_get_systicks_to_ms();
    uint32_t period_ms = 30 * 1000;

    // charging state
    if( tick_ms < timer_next )
    {
        return (1 + timer_next - tick_ms);
    }
    timer_next = ql_rtos_get_systicks_to_ms() + period_ms;

    qr_index++;
    if ( qr_index & 0x01 )
    {
        disp_set_qrcode( QR_CONTENT2, strlen( QR_CONTENT2 ) );
    }
    else
    {
        disp_set_qrcode( QR_CONTENT, strlen( QR_CONTENT ) );
    }

    return period_ms;
}

