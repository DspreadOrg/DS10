/**
  ******************************************************************************
  * @file    lowpower_mgmt.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   �͹��Ĺ��� func file for C header
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
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_power.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "lowpower_mgmt.h"

#define LOG_INFO(...)           do{printf("[INFO LPM]: "); printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
static uint32_t timer_last_busy = 0;
static int g_lpm_lock = -1;
static int is_lpm_lock = 0;
ql_task_t lpm_task_ctrl_thread = NULL;
static ql_mutex_t s_active_lock = NULL;
lpm_lock_t g_lpm_locks;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void lpm_set(uint8_t lock_type,uint8_t value)
{
    if( s_active_lock )
    {
        ql_rtos_mutex_lock( s_active_lock, 0xFFFFFFFF );

        switch( lock_type )
        {
            case LPM_LOCK_BUTTON:
                g_lpm_locks.bits.button = value;
                break;

            case LPM_LOCK_MQTT:
                g_lpm_locks.bits.mqtt = value;
                break;

            case LPM_LOCK_TTS:
                g_lpm_locks.bits.tts = value;
                break;

            case LPM_LOCK_INIT:
                g_lpm_locks.bits.init = value;
                break;

            case LPM_LOCK_WIFI:
                g_lpm_locks.bits.wifi = value;
                break;

            case LPM_LOCK_NETWORK:
                g_lpm_locks.bits.network = value;
                break;
        }

        if( g_lpm_locks.value )
        {
            timer_last_busy = ql_rtos_get_systicks_to_ms();
            if( !is_lpm_lock )
            {
                LOG_INFO( "%s: set lpm lock,bits 0x%04X\n", __func__,g_lpm_locks.value );
                ql_lpm_wakelock_lock( g_lpm_lock );
                is_lpm_lock = 1;
            }
        }
        else
        {
            if( is_lpm_lock )
            {
                LOG_INFO( "%s: release lpm lock,ready to enter lowpower mode...\n", __func__ );
                ql_lpm_wakelock_unlock( g_lpm_lock );
                is_lpm_lock = 0;
            }
        }

        ql_rtos_mutex_unlock( s_active_lock );
    }
}

void lpm_task(void *pvParameters)
{
    while(1)
    {
        if( !g_lpm_locks.value && is_lpm_lock )
        {
            //if( ql_rtos_get_systicks_to_ms() > (timer_last_busy + 10 * 1000) )
            {
                LOG_INFO( "%s: release lpm lock\n", __func__ );
                ql_lpm_wakelock_unlock( g_lpm_lock );
                is_lpm_lock = 0;
            }
        }
        ql_rtos_task_sleep_ms(3000);
    }
}

void lpm_init(void)
{
    g_lpm_locks.value = 0;
    ql_rtos_mutex_create(&s_active_lock);

    g_lpm_lock = ql_lpm_wakelock_create( "lpm_lock", sizeof( "lpm_lock" ) );
    if ( g_lpm_lock <= 0 )
    {
        LOG_INFO( "ql_lpm_wakelock_create, lpm_fd = %d\n", g_lpm_lock );
        return;
    }

    lpm_set(LPM_LOCK_NETWORK,1);
    ql_autosleep_enable( 1 );

#if 0
    if (ql_rtos_task_create(&lpm_task_ctrl_thread,
                        3*1024,
                        100,
                        "lpm_task",
                        lpm_task,
                        NULL) != 0) {
        usb_log_printf("--thread create error\n");
    }
#endif
}


