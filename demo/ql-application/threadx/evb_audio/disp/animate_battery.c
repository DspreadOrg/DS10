/**
  ******************************************************************************
  * @file    animate_charge.c
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
#include "ql_rtc.h"
#include "prj_common.h"
#include "systemparam.h"
#include "public_api_interface.h"
#include "lvgl.h"
#include "res.h"
#include "terminfodef.h"
#include "animate.h"
#include "disp_port.h"

#define LOG_INFO(...)           do{printf("[ANM]: "); printf(__VA_ARGS__);}while(0)

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

static void bat_state_update(uint8_t force_update)
{
    static uint8_t pre_bat_level = 0xff;
    uint8_t bat_level = 3;
    if( TermInfo.BatRemain >= 80 )
    {
        bat_level = 4;
    }
    else if( TermInfo.BatRemain > 50 )
    {
        bat_level = 3;
    }
    else if( TermInfo.BatRemain > 30 )
    {
        bat_level = 2;
    }
    else if( TermInfo.BatRemain >= 10 )
    {
        bat_level = 1;
    }
    else
    {
        bat_level = 0;
    }

    if( !force_update && (pre_bat_level == bat_level) )
    {
        return;
    }
    LOG_INFO("%s: switch bat state to %d\n",__func__,bat_level);
    pre_bat_level = bat_level;

    disp_set_bat_level(bat_level);
}

int animate_battery(void)
{
    static uint32_t timer_next = 0;
    static uint8_t anm_state = 0;
    static uint8_t charge_status = 0;
    uint32_t tick_ms = ql_rtos_get_systicks_to_ms();
    uint32_t period_ms = 500;

    // update charge icon
    if( charge_status != TermInfo.Charge )
    {
        // wakeup display
        charge_status = TermInfo.Charge;
        disp_set_charge(TermInfo.Charge);
        if( TermInfo.Charge )
        {
            // keep disp on when charging
            disp_onoff_request(1,DISP_HOLDON_FOREVER);
            disp_clock_enable(1);
        }
        else
        {
            bat_state_update(1);
            disp_onoff_request(1,DISP_HOLDON_MS);
            disp_clock_enable(0);
        }
        anm_state = 0;
    }

    if( !TermInfo.Charge )
    {
        // no charge,update bat level disp
        bat_state_update(0);
        return 30000;
    }

    // charging state
    if( tick_ms < timer_next )
    {
        return (1 + timer_next - tick_ms);
    }
    timer_next = ql_rtos_get_systicks_to_ms() + period_ms;

    if( TermInfo.ChargeFull )
    {
        if( anm_state != 0xff )
        {
            disp_set_bat_level(4);
            anm_state = 0xff;
        }
        return period_ms;
    }
    else if( TermInfo.BatRemain <= 30 )
    {
        switch ( anm_state )
        {
            case 0:
                disp_set_bat_level(0);
                anm_state++;
                break;

            case 1:
                disp_set_bat_level(1);
                anm_state++;
                break;

            case 2:
                disp_set_bat_level(2);
                anm_state++;
                break;

             case 3:
                 disp_set_bat_level(3);
                anm_state++;
                break;

             case 4:
                 disp_set_bat_level(4);
                anm_state = 0;
                break;

             default:
                 anm_state = 0;
                 break;
        }
    }
    else if( TermInfo.BatRemain <= 50 )
    {
        switch(anm_state)
        {
            case 0:
                disp_set_bat_level(1);
                anm_state++;
                break;

            case 1:
                disp_set_bat_level(2);
                anm_state++;
                break;

             case 2:
                 disp_set_bat_level(3);
                anm_state++;
                break;

             case 3:
                 disp_set_bat_level(4);
                anm_state = 0;
                break;

             default:
                 anm_state = 0;
                 break;
        }
    }
    else if( TermInfo.BatRemain <= 70 )
    {
        switch(anm_state)
        {
            case 0:
                disp_set_bat_level(2);
                anm_state++;
                break;

             case 1:
                 disp_set_bat_level(3);
                anm_state++;
                break;

             case 2:
                 disp_set_bat_level(4);
                anm_state = 0;
                break;

             default:
                 anm_state = 0;
                 break;
        }
    }
    else if( TermInfo.BatRemain < 91 )
    {
        switch(anm_state)
        {
            case 0:
                disp_set_bat_level(3);
                anm_state++;
                break;

            case 1:
                disp_set_bat_level(4);
                anm_state = 0;
                break;

            default:
                anm_state = 0;
                break;
        }
    }
    else // 100%
    {
        switch(anm_state)
        {
            case 0:
                disp_set_bat_level(4);
                anm_state++;
                break;

            default:
                anm_state = 0;
                break;

        }
    }

    return period_ms;
}

