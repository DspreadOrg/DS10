/**
  ******************************************************************************
  * @file    wifi.C
  * @author  Xu
  * @version V1.0.0
  * @date    2021/08/16
  * @brief   wifi func file for C code
  * @code    GBK编码
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "gpio.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "ext_wifi.h"
#include "tts_yt_task.h"
#include "wifi.h"

#define LOG_DBG(...)            //do{printf("[DBG WIFI]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{printf("[INFO WIFI]: "); printf(__VA_ARGS__);}while(0)
//#define _LOG_INFO(...)          do{printf(__VA_ARGS__);}while(0)

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

// set wifi ap name

static uint32_t Tsec=0;
void ModeInfoPlay(void);

int WifiParaSetSav(char const * ssid, char const * psk)
{
	struct sysparam * ysw1_sysparam;
	ysw1_sysparam = sysparam_get();
	memset(&ysw1_sysparam->wlan_sta_param, 0, sizeof(ysw1_sysparam->wlan_sta_param));
	memcpy((char *)ysw1_sysparam->wlan_sta_param.psk, psk, strlen(psk));
	memcpy((char *)ysw1_sysparam->wlan_sta_param.ssid,ssid, strlen(ssid));
	ysw1_sysparam->wlan_sta_param.ssid_len = strlen(ssid);

	sysparam_save();
	printf("wifi setting success\n");

	return 0;
}

int ApModeInfoPlay(void)
{
	short group[4];
	int cnt;
	if (TermInfo.buttonAP==2)
	{
		if (ql_rtos_get_systicks_to_s()>Tsec)
		{
			Tsec=ql_rtos_get_systicks_to_s()+60;
			
			//tts_play_set(AudioXianzswifiappwms,AudioXianzswifiappwmsLen,FixAudioTypeDef);
			//tts_play_set(AudioShuangjgnjqhzklpwms,AudioShuangjgnjqhzklpwmsLen,FixAudioTypeDef);
			//tts_play_set_idx(AUD_ID_NET_MODE_WIFI_AP,0,0);
			//tts_play_set_idx(AUD_ID_HOW_WIFI_TO_AIRKISS,0,0);
			cnt=0;
			group[cnt++]=AUD_ID_NET_MODE_WIFI_AP;
			group[cnt++]=AUD_ID_HOW_WIFI_TO_AIRKISS;
			if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
				|| ( sysparam_get( )->NetChanlLTE == GPRS_BAKE_MODE )
				|| ( sysparam_get( )->NetChanlLTE == GPRS_MODE ) )
			{
				//tts_play_set(AudioHuo,AudioHuoLen,FixAudioTypeDef);
				//tts_play_set(AudioChangagnjqhzgprswlms,AudioChangagnjqhzgprswlmsLen,FixAudioTypeDef);
				//tts_play_set_idx(AUD_ID_OR,0,0);
				//tts_play_set_idx(AUD_ID_HOW_WIFI_TO_GPRS,0,0);
				group[cnt++]=AUD_ID_OR;
				group[cnt++]=AUD_ID_HOW_WIFI_TO_GPRS;
			}
			tts_play_set_group(group,cnt,0,0);
		}
		//	LOG_INFO( "%s: ===================%d\n", __func__ ,TermInfo.buttonAP);
		return 0;
	}
	//LOG_INFO( "%s: ===================%d\n", __func__ ,TermInfo.buttonAP);

	return -1;
}

int AirkissModeInfoPlay(void)
{
	short group[4];
	int cnt;

	if (TermInfo.buttonAP==1)
	{
		if (ql_rtos_get_systicks_to_s()>Tsec)
		{
			Tsec=ql_rtos_get_systicks_to_s()+60;
			
			//tts_play_set(AudioXianzswifiklpwms,AudioXianzswifiklpwmsLen,FixAudioTypeDef);
			//tts_play_set_idx(AUD_ID_NET_MODE_WIFI_AIRKISS,1,1);
			cnt=0;
			group[cnt++]=AUD_ID_NET_MODE_WIFI_AIRKISS;
			if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
				|| ( sysparam_get( )->NetChanlLTE == GPRS_BAKE_MODE )
				|| ( sysparam_get( )->NetChanlLTE == GPRS_MODE ) )
			{
				//tts_play_set(AudioChangagnjqhzgprswlms,AudioChangagnjqhzgprswlmsLen,FixAudioTypeDef);
				//tts_play_set_idx(AUD_ID_HOW_WIFI_TO_GPRS,0,0);
				group[cnt++]=AUD_ID_HOW_WIFI_TO_GPRS;
			}
			tts_play_set_group(group,cnt,0,0);
		}
		return 0;
	}

	return -1;
}

// set wifi network
char WifiConfigSet(void)
{
    int ret=-1;

    uint8_t softap_ssid[24];
    const dev_config_t *pdevconf = get_device_config( );

    memset( softap_ssid, 0x00, sizeof( softap_ssid ) );
    snprintf( softap_ssid, sizeof( softap_ssid ) - 4, "YM-%s-", pdevconf->devname );
    strcat( (char*) softap_ssid, ( sysparam_get( )->device_SN ) + ( strlen( sysparam_get( )->device_SN ) - 3 ) );

    Tsec=0;
    while (ret!=0)
    {
        printf("wifi setting ap mode\n");
        if (ApModeInfoPlay()) break;//模式提醒控制
//        ret = wifi_ap_task(softap_ssid,NULL,NULL);//无60秒模式提醒
        ret = wifi_ap_cfg(softap_ssid,"192.168.1.1",NULL,ApModeInfoPlay);//有60秒模式提醒
		if(ret==0) return ret;
    }


    ret = -1;
    Tsec=0;
    while(ret!=0)
    {
        printf("wifi setting airkiss\n");
        if (AirkissModeInfoPlay()) break;//模式提醒控制
        //ret=wifi_airkiss_task(60000,NULL);//无60秒模式提醒
        ret=wifi_airkiss_cfg(60000,NULL,AirkissModeInfoPlay);//有60秒模式提醒
		if(ret==0) return ret;
    }
    usb_log_printf("%s_%d === \n", __func__, __LINE__);
	return ret;
}

int WifiCtrlInit(void)
{
	int ret=-1;
	int ii;
    extern char BootOver;
    char pre_bootover;

    LOG_INFO("%s: start\n",__func__);
    if (sysparam_get_device_type() == 0)
    {
    	usb_log_printf("%s: ex_wifi is not exist,skip ex_wifi init\n",__func__);
        return ret;
    }

    const dev_config_t *pdevconf = get_device_config();
    if( (pdevconf->pins->wifi_power.gpio_num <= GPIO_PIN_NO_NOT_ASSIGNED)
         || (pdevconf->pins->wifi_power.gpio_num >= GPIO_PIN_NO_MAX) )
    {
        LOG_INFO( "%s: wifi power pin is not config, wifi is not exist\n", __func__ );
       // sysparam_set_device_type( 0 );
       // sysparam_save( );

       // return ret;
    }

	WifiSocketInit();//开机仅操作一次
	serial_init();//开机仅操作一次
	if (0)//get_device_config()->id==ID_CS35)
	{
		ql_uart_close(QL_BT_UART_PORT);
		serial_start(QL_BT_UART_PORT, QL_UART_BAUD_115200, QL_UART_DATABIT_8, QL_UART_PARITY_NONE, QL_UART_STOP_1, QL_FC_HW);
	}
	else
	{
		ql_uart_close(QL_MAIN_UART_PORT);
		serial_start(QL_MAIN_UART_PORT, QL_UART_BAUD_115200, QL_UART_DATABIT_8, QL_UART_PARITY_NONE, QL_UART_STOP_1, QL_FC_NONE);		
	}


	for(ii=0;ii<3;ii++)
	{
		gpio_wifi_power_onoff(0);
			gpio_wifi_en_Set(0);
		ql_rtos_task_sleep_ms(50);
		pre_bootover = BootOver;
		gpio_wifi_power_onoff(1);
					ql_rtos_task_sleep_ms(2);
			gpio_wifi_en_Set(1);
#if 0
        ql_rtos_task_sleep_ms(1000);
#else   // 降低启动耗时

        for(int t = 0; t < 120; t++ )
        {
            if( pre_bootover != BootOver )
            {
                break;
            }
            ql_rtos_task_sleep_ms(20);
        }
#endif
		if (Ext_wifi_reBoot())
		{
			//ret = serial_SendATCmd("AT\r\n", 4,AT_back_handler,NULL,MODULE_CMD_TIMEOUT_NORMAL);
			//if (ret==RIL_AT_SUCCESS)
			if (0==WifiAtChk())
			{
				ret=0;
				LOG_INFO("%s: ex_wifi check ok\n",__func__);
				break;
			}
		}
	}

    if (BootOver == 0)  // check timeout,no wifi
    {
        LOG_INFO("===========Ext_Wifi_not found!===========\r\n");
        if (sysparam_get_device_type() == -1 )
        {
            sysparam_set_device_type(0);
            sysparam_save();
        }
    }
    else
    {
		WifiGetVer();
        // wifi exist
        LOG_INFO("===========Ext_Wifi_Ver:%s===========\r\n",sysparam_get()->WifiVer);
        if (sysparam_get_device_type() != 1)
        {
            sysparam_set_device_type( 1 );
            sysparam_save( );
        }
    }
	
    if(get_factory_test_status()==0)
		Ext_wifi_Off();
	
	return ret;
}

