/**
  ******************************************************************************
  * @file    animate.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD 78x64 func file for C code
  * @code    GBK±àÂë£¬
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
#include "ql_power.h"
#include "prj_common.h"
#include "systemparam.h"
#include "public_api_interface.h"
#include "lvgl.h"
#include "res.h"
#include "terminfodef.h"
#include "animate.h"
#include "disp_port.h"

#define LOG_INFO(...)           do{printf("[INFO ANM]: "); printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
static ql_task_t animate_task_ctrl = NULL;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


static int animate_service(void)
{
    int min_period_ms = 30 * 1000;
    int period_ms;
    period_ms = animate_battery();
    min_period_ms = (period_ms < min_period_ms)?period_ms:min_period_ms;
    period_ms = animate_signal();
    min_period_ms = (period_ms < min_period_ms)?period_ms:min_period_ms;
#if (DEV_FG7864_SUPPORT) || (DEV_F240320_SUPPORT) || (DEV_F480320_SUPPORT)
    if ( TermInfo.disp.fg7864
     || TermInfo.disp.f240320 )
    {
        period_ms = animate_qrcode();
        min_period_ms = (period_ms < min_period_ms)?period_ms:min_period_ms;
    }
#endif
    return min_period_ms;
}

static void disp_animate_task(void *pvParameters)
{
    LOG_INFO("%s: start\n",__func__);
    uint32_t wait_ms = -1;
    uint32_t min_wait_ms;
    while ( 1 )
    {
        min_wait_ms = 3 * 1000;
        wait_ms = animate_service( );
        if ( wait_ms < min_wait_ms )  min_wait_ms = wait_ms;

        ql_rtos_task_sleep_ms(min_wait_ms);
    }
}

// run animate task
void animate_task_init(void)
{
    if( animate_task_ctrl )
    {
        LOG_INFO("%s: task is init\n",__func__);
        return;
    }

    if (ql_rtos_task_create(&animate_task_ctrl,
                            5 * 1024,
                            100,
                            "animate task",
                            disp_animate_task,
                            NULL) != 0)
    {
        LOG_INFO( "%s: thread create error\n", __func__ );
    }
}
