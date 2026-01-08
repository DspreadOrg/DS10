/**
  ******************************************************************************
  * @file    animate_charge.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   LCD 78x64 func file for C code
  * @code    GBK编码，
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

#define LOG_DBG(...)           //do{printf("[ANM DBG]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           //do{printf("[ANM INFO]: "); printf(__VA_ARGS__);}while(0)
#define _LOG_INFO(...)          //do{printf(__VA_ARGS__);}while(0)

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

// update signal disp
int animate_signal(void)
{
    //static uint32_t timer_next = 0;
    static uint8_t pre_signal_level = 0;
    static float signal_samples[5] = {0,};
    static uint8_t sample_cnt = 0;
    static char netmode = 0;
    uint32_t period_ms = 2500;
	static uint32_t timer_next = 0;
	if( netmode != TermInfo.NetMode )
	{
		netmode = TermInfo.NetMode;
		memset(&signal_samples,0,sizeof(signal_samples));
		pre_signal_level = 0;
//		LOG_INFO("	 pre_signal_level= %d\n",pre_signal_level);
		sample_cnt = 0;
	}

#if 1
	else
	{
	    // 控制刷新频率
	    uint32_t tick_ms = ql_rtos_get_systicks_to_ms( );
	    if ( tick_ms < timer_next )
	    {
	        return period_ms;
	    }
	    timer_next = ql_rtos_get_systicks_to_ms( ) + period_ms;
	}
#endif



    if( (TermInfo.NetMode == GPRS_MODE) || (TermInfo.NetMode == GPRS_BAKE_MODE) )
    {
        /*
         * a. 极好点： RSRP>-85dBm； SINR>25
         * b. 好点： RSRP=-85～-95dBm；SINR:16-25
         * c. 中点： RSRP=-95～-105dBm；SINR:11-15
         * d. 差点： RSRP=-105～-115dBm；SIN:3-10
         * e. 极差点： RSRP<-115dB;SINR<3
         */
        if( TermInfo.NetStatBak == NET_DEVICE_STATE_CONNECTED )
        {
            QL_NW_SIGNAL_STRENGTH_INFO_T signal_info;
            uint8_t signal_level = pre_signal_level;
            if (0 == ql_nw_get_signal_strength(&signal_info))
            {
                for(int i = 0; i < 4; i++ )
                {
                    signal_samples[i] = signal_samples[i+1];
                }
                signal_samples[4] = signal_info.LTE_SignalStrength.rsrp;
                sample_cnt++;
                if( sample_cnt > 5 )
                {
                    sample_cnt = 5;
                }
                int emp_signal = signal_info.LTE_SignalStrength.rsrp;
                if( sample_cnt >= 5 )
                {
                    emp_signal = signal_samples[0] * 30 / 100
                                 + signal_samples[1] * 30 / 100
                                 + signal_samples[2] * 15 / 100
                                 + signal_samples[3] * 15 / 100
                                 + signal_samples[4] * 10 / 100;
                }

                LOG_INFO("%s: %d,%d,%d,%d,%d,result=%d\n",__func__,(int)signal_samples[0],(int)signal_samples[1],
                        (int)signal_samples[2],(int)signal_samples[3],(int)signal_samples[4],emp_signal);
                if( emp_signal >= -95 )
                {
                    signal_level = 4;
                }
                else if(signal_info.LTE_SignalStrength.rsrp >= -105 )
                {
                    signal_level = 3;
                }
                else if(signal_info.LTE_SignalStrength.rsrp >= -115 )
                {
                    signal_level = 2;
                }
                else
                {
                    signal_level = 1;
                }
//				LOG_INFO("   signal_level= %d   pre_signal_level= %d\n",signal_level,pre_signal_level);
//                if( signal_level != pre_signal_level )//每次都更新信号状态
                {
                    LOG_INFO("%s: current rsrp %d, update lte signal level to %d\n",__func__,
                            signal_info.LTE_SignalStrength.rsrp,signal_level);
                    pre_signal_level = signal_level;
                    disp_set_gprs_signal_level(signal_level);
                    timer_next = ql_rtos_get_systicks_to_ms( ) + 5000;
                }
            }
        }
        else
        {
            timer_next = ql_rtos_get_systicks_to_ms( ) + 2000;
        }
    }
    else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
    {
        // not support
        int current_level;
        uint8_t signal_level = pre_signal_level;
        if( TermInfo.NetStatBak == NET_DEVICE_STATE_CONNECTED )
        {
            current_level = GetWifiSignalLevel();
            LOG_DBG("%s: wifi signal level %d\n",__func__,current_level);

            for ( int i = 0; i < 4; i++ )
            {
                signal_samples[i] = signal_samples[i + 1];
            }
            signal_samples[4] = current_level;
            sample_cnt++;
            if ( sample_cnt > 5 )
            {
                sample_cnt = 5;
            }
            int emp_signal = current_level;
            if( sample_cnt >= 5 )
            {
                emp_signal = signal_samples[0] * 30 / 100
                             + signal_samples[1] * 30 / 100
                             + signal_samples[2] * 15 / 100
                             + signal_samples[3] * 15 / 100
                             + signal_samples[4] * 10 / 100;
            }
            LOG_INFO("%s: %d,%d,%d,%d,%d,result=%d\n",__func__,(int)signal_samples[0],(int)signal_samples[1],
                    (int)signal_samples[2],(int)signal_samples[3],(int)signal_samples[4],emp_signal);
            if( current_level >= -40 )
            {
                signal_level = 4;
            }
            else if( current_level >= -50 )
            {
                signal_level = 3;
            }
            else if( current_level >= -70 )
            {
                signal_level = 2;
            }
            else //if( current_level >= 20 )
            {
                signal_level = 1;
            }

//            if ( signal_level != pre_signal_level )//切换模式太频繁会导致信号值没更新，变成0，所以注释掉改成实时更新
            {
                LOG_INFO("%s: current rssi %d,update wifi signal level to %d\n",__func__,current_level,signal_level);
                pre_signal_level = signal_level;
                disp_set_wifi_signal_level(signal_level);
                timer_next = ql_rtos_get_systicks_to_ms( ) + 5000;
            }
        }
        else
        {
            timer_next = ql_rtos_get_systicks_to_ms( ) + 2000;
        }

    }

    return period_ms;
}

