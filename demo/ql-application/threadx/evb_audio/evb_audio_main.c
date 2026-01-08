#include <stdio.h>
#include <string.h>
#include "ql_application.h"
#include "ql_rtos.h"
#include "ql_dev.h"
#include "ql_data_call.h"
#include "ql_sim.h"
#include "ql_nw.h"
#include "sockets.h"
#include "netdb.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_spi.h"
#include "ql_spi_nor.h"
//#include "ql_atcmd.h"
#include "ql_audio.h"
#include "conf_devtype.h"
#include "prj_common.h"
#include "mqtt_aliyun.h"
#include "tts_yt_task.h"
#include "httplayerjson.h"
#include "systemparam.h"
#include "gpio_button_task.h"
#include "button.h"
#include "drv_bat.h"
#include "led.h"
#include "gpio.h"
#include "bat.h"
#include "terminfodef.h"
#include "res.h"
#include "ql_fs.h"
#include "module_lte.h"
#include "record_store.h"
#include "record_play.h"
#include "play_receipt.h"
#include "ext_wifi.h"
#include "ext_wifi_socket.h"
#include "wifi.h"
#include "ql_api_map_common.h"
#include "public_api_interface.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"


#define LOG_DBG(...)            do{usb_log_printf("[DBG MAIN]: "); usb_log_printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{usb_log_printf("[INFO MAIN]: "); usb_log_printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
//�汾�ų���20�ַ���
char firmware_version[50] = SOFTVER;
char g_kernel_version[20] = "5.x";

StructTermInfo TermInfo = {0};
static unsigned char poweron_switch_modem_min_fun = 0;
static char WifiSetOK = 0;
ql_task_t app_task_ctrl_thread = NULL;
#define APP_TASK_CTRL_THREAD_STACK_SIZE	(10 * 1024)

//����ʱ�䳬������ʱ��֮���������ʾ
#define NetFailAudioTimeDef	60

extern unsigned char mqtt_first_connet;
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
extern uint8_t isEnterFactorymode();
/******************************************************************************
��Զ��:
AT+LOG=15,13����ATָ����Թص��ر�AT+DIAG������USB�˿�

��Զ��:
AT+LOG=15,13
����ATָ����Թر�AT+DIAG������USB�˿ڣ�ֻ����USB CDC�˿�

AT+LOG=15,12
����ATָ����Ի�ԭAT+DIAG������USB�˿�

Ӧ�ÿ���ͨ��ql_atcmd_send_sync���APIȥ��������˵��ATָ��ʵ�ֶ�Ӧ�Ĺ���

////�����ӳ����⣬�л���վ�㷨���ɻ�վ���ȸ�Ϊ�ź�ǿ����
@Ʈ��?@A����?
at+medcr=0,79,1��at+cfun=0��at+cfun=1��
������籣�棬������Ƶ����ģ����Ч��

ԭ��û�ṩ�ĵ���
at+medcr=0,79,1   //����
at+medcr=0,79,0  //�رա�Ĭ����0
*******************************************************************************/
// �ָ���������
void device_refactory(void)
{
	struct sysparam *psysconfig = sysparam_get();
    
	tts_play_set_idx(AUD_ID_RECOVER_FACTORY,0,0);

	ql_rtos_task_sleep_ms(2000);

	memset(&psysconfig->wlan_sta_param, 0, sizeof(psysconfig->wlan_sta_param));
    
    #ifndef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	memset(psysconfig->product_key, 0, SYSINFO_PSK_LEN_MAX-1);
	memset(psysconfig->device_name, 0, SYSINFO_PSK_LEN_MAX-1);
	memset(psysconfig->device_secret, 0, SYSINFO_PSK_LEN_MAX-1);
    #endif
    
	psysconfig->register_state = 0;
	psysconfig->volume = 3;
	psysconfig->NetChanlLTE = GPRS_BAKE_MODE;
	sysparam_save();

    TermInfo.Repeat = NULL;
    MqttExit();
	
	Record_Manage_Clean();
	AudioPlayHalt();
	ql_rtos_task_sleep_ms(800);
	ql_power_reset();
}
void keyboard_input_callback(int input_type, void *param )
{
    static uint8_t last_type = 0;
    char *playlist = NULL;
    int64_t value = *((int64 *)param);

    switch(input_type)
    {
        case KB_INPUT_OK:
            tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
            break;
        case KB_INPUT_ERR:
            tts_play_set_idx(AUD_ID_KEY_ERR_BEEP,1,1);
            break;

        case KB_INPUT_CLEAR:
            last_type = 0;
            break;

        case KB_INPUT_SET:
            break;
        case KB_INPUT_REPEAT:
            if( last_type )
            {
                input_type = last_type;
            }
            else
            {
                break;
            }
        case KB_INPUT_RETURN:
        case KB_INPUT_PAY:
            break;
    }// end switch

    if( playlist )
    {
        free(playlist);
    }

}

void AtBlackCellOnOff(int set)
{
	//set=0~2
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];

	sprintf(atcmd,"AT+QBLACKCELLCFG=%d\r\n",set>0?1:0);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	LOG_INFO("=============%s , line %d,%s===========ret %d\n", __func__, __LINE__,atcmd,ret);
	sprintf(atcmd,"AT+QBLACKCELLCFG?\r\n");
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	LOG_INFO("=============%s , line %d,%s,ret %d,%s===========\n", __func__, __LINE__,atcmd,ret,resp_buf);
}
void bus_clk_52M(void)
{
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];
	sprintf(atcmd,"at*regrw=w,d4090118,1\r\n");
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s===========ret %d\n", __func__, __LINE__,atcmd,ret);
}

void start(void)
{
    AtBlackCellOnOff(1);
	bus_clk_52M();
	disp_ui_init();
	gpio_led_task_init(); 
	TermLedShow(TERM_INIT_START);
	drv_bat_init();
    
    if (sysparam_get()->NetChanlLTE == WIFI_MODE)
    {
        poweron_switch_modem_min_fun = 1;
    }
    else
    {
        LTE_CAT1_init();
    }
    
	tts_play_init(); // tts 
	ResTabChk();

	gpio_button_ctrl_init();
	button_task_init();
    
	if(sysparam_get()->uart_log == 2)
	{
		usb_log_uart_init(); // 
		tts_play_set_idx(AUD_ID_LOG_MODE_USB,0,0);
		while(!usb_log_uart_start())
			ql_rtos_task_sleep_ms(100);
	}
	else if (sysparam_get()->uart_log == 3)
	{
		usb_log_uart_init(); // 
	}

	Record_Manage_Init();
	play_receipt_init();
	Record_Read_MSGID(0);

	if (TermInfo.LowBat>0)
	{
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,0,0);
	}

	LOG_INFO("LedModeSet Blink\n");

#if DEV_KEYBOARD_SUPPORT
	if( TermInfo.disp.tm1721 )
	{
		keyboard_init();
		keyboard_set_input_callback(keyboard_input_callback);
	}
#endif
    
	lpm_init();

	TermLedShow(TERM_INIT_END);
}

int FuncWifiErrRepeat(void)
{
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		disp_set_net_connect_error(1);
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		return -1;
	}
	else if (TermInfo.NetStatBak != NET_DEVICE_STATE_CONNECTED) 
	{
		int state=NET_DEVICE_STATE_DISCONNECTED;
		if (Wifi_Que_stat(&state)==0)
		{
			NetStat = ( state & 0x10 ) ? 1 : 0;
			WifiSetOK = ( state & 0x10 ) ? 1 : 0;

		}

		LOG_INFO("FuncWifiErrRepeat wifi disconnect\n");
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
		TermLedShow(TERM_NET_DIS);
		disp_set_net_connect_error(1);
		LOG_INFO("net fail time %d,cur %d\n",TermInfo.FailTime,ql_rtos_get_systicks_to_s());
		if(ql_rtos_get_systicks_to_s()>(TermInfo.FailTime+NetFailAudioTimeDef))
		{
			TermInfo.PlayNetSucc=0;
			tts_play_set_idx(AUD_ID_NET_CONNECT_FAIL,0,0);
		}
		return -3;
	}
	TermInfo.Repeat=NULL;
	return 0;
}

int FuncModuleErrRepeat(void)
{
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		disp_set_net_connect_error(1);
		TermInfo.RepTim = ql_rtos_get_systicks_to_s()+30;
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		return -1;
	}
	else if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
	{
		TermLedShow(TERM_NET_DIS);
		disp_set_net_connect_error(1);
		TermInfo.RepTim = ql_rtos_get_systicks_to_s()+30;
		LOG_INFO("net fail time %d,cur %d\n",TermInfo.FailTime,ql_rtos_get_systicks_to_s());
		if(ql_rtos_get_systicks_to_s()>(TermInfo.FailTime+NetFailAudioTimeDef))
		{
			TermInfo.PlayNetSucc=0;
			tts_play_set_idx(AUD_ID_NET_CONNECT_FAIL,0,0);
		}
		return -3;
	}
	
	TermInfo.Repeat=NULL;
	return 0;
}

int FuncWifiRetryRepeat(void)
{
	unsigned int address = 0;
	static int index=0;
	int state=NET_DEVICE_STATE_DISCONNECTED;

	if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
		return -1;

	if (Wifi_Que_stat(&state)==0)
	{
		NetStat = ( state & 0x10 ) ? 1 : 0;
		WifiSetOK = ( state & 0x10 ) ? 1 : 0;

	}

	if (!TermInfo.SntpOk)
	{
		LOG_INFO("enter %s , line %d,SntpOk %d\n", __func__, __LINE__, TermInfo.SntpOk);
		
		Wifi_SntpTime("cn.ntp.org.cn");//�����й�ʱ��
		if(Wifi_SntpTime(NULL) ) // ͬ��ʱ��ʧ��  ����task5sͬ��һ��ֱ���ɹ�
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;
		else 
			TermInfo.SntpOk=1;

		LOG_INFO("enter %s , line %d, read_time: %d\n", __func__, __LINE__, read_rtc_time());
	}
	
	TermInfo.RepTim =ql_rtos_get_systicks_to_s()+60;
	if (Wifi_GetHostByName("www.aliyun.com", &address) != 0) 
	{
		index++;
		LOG_INFO("invalid ping host.\n");
		if (index>6)
		{
			index=0;
			disp_set_net_connect_error(1);
			tts_play_set_idx(AUD_ID_NET_FAULT,0,0);
		}
		return -2;
	}
	LOG_INFO("%s_%d ===ipaddr:%d.%d.%d.%d\n", __func__, __LINE__,(address>>0)&0xff,(address>>8)&0xff,(address>>16)&0xff,(address>>24)&0xff);

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		TermLedShow(TERM_PARAM_ERR);
		disp_set_net_connect_error(1);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		return -3;
	}
	
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if (0)
#else
	// if(!(sysparam_get()->register_state)) 
	if (0)
#endif
	{
		LOG_INFO("enter %s , line %d,register_state %d\n", __func__, __LINE__,sysparam_get()->register_state);
		if (0 != device_sign(TermInfo.NetMode))
		{
			TermLedShow(TERM_REG_FAIL);
			disp_set_net_connect_error(1);
			tts_play_set_idx(AUD_ID_ACTIVE_FAIL,0,0);
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
			return -3;
		}
		else
		{
			tts_play_set_idx(AUD_ID_ACTIVE_SUCESS,0,0);
			sysparam_get()->register_state = 1;
			sysparam_save();
		}
	}

	if ((!TermInfo.MqttIsRuning)&&(!TermInfo.OTAMode))
	{
		disp_service_connecting();
		start_Mqtt_task();
	}

	if ((!TermInfo.SntpOk))
	{
		TermInfo.RepTim = ql_rtos_get_systicks_to_s( ) + 10;
		return -1;
	}
	
	return 0;
}

int FuncLteRetryRepeat(void)
{
	int ret;
	if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED) 
		return -1;

	if(!TermInfo.SntpOk)
	{
		LOG_INFO("enter %s , line %d,SntpOk %d\n", __func__, __LINE__, TermInfo.SntpOk);
		if(sntp_get_net_time(NULL) ) //
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;
		else 
			TermInfo.SntpOk=1;

		LOG_INFO("enter %s , line %d, read_time: %d\n", __func__, __LINE__, read_rtc_time());
	}

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
		TermLedShow(TERM_PARAM_ERR);
		disp_set_net_connect_error(1);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		return -1;
	}

#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if (0)
#else
	// if(!(sysparam_get()->register_state)) 
	if (0)
#endif
	{
		LOG_INFO("enter %s , line %d,register_state %d\n", __func__, __LINE__,sysparam_get()->register_state);
		ret = device_sign(TermInfo.NetMode);
		LOG_INFO("enter %s , line %d,ret = %d\n", __func__, __LINE__,ret );
		if (0 != ret)
		{
			TermLedShow(TERM_REG_FAIL);
			disp_set_net_connect_error(1);
			tts_play_set_idx(AUD_ID_ACTIVE_FAIL,0,0);
			TermInfo.RepTim =ql_rtos_get_systicks_to_s()+30;
			return -3;
		}
		else
		{
			tts_play_set_idx(AUD_ID_ACTIVE_SUCESS,0,0);
			sysparam_get()->register_state = 1;
			sysparam_save();
		}
	}

    TermInfo.RepTim =ql_rtos_get_systicks_to_s()+5;

	if ((!TermInfo.MqttIsRuning)&&(!TermInfo.OTAMode))
	{
	    disp_service_connecting();
	    start_Mqtt_task();
	}

	
	if ((!TermInfo.SntpOk))
	{
		TermInfo.RepTim =ql_rtos_get_systicks_to_s()+10;
		return -1;
	}	
	
	return 0;
}

#if 0
char AudioPlayLevelFlag=0;
void AudioPlayLevel(void)
{
	if (AudioPlayLevelFlag)
	{
		char audio[50];
		AudioPlayLevelFlag=0;
		int level=-1;
					
		if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
			level=GetWifiSignalLevel();
		else
			level=GetGsmSignalLevel();
					
		if (level<=0)
		{
			tts_play_set_idx(AUD_ID_GET_SIGNAL_FAIL,1,1);
		}
		else
		{
			short group[10];
			short * ptr;
			int ret;
			ptr=group;
			*ptr++=AUD_ID_CUR_SIGNAL;
			ret=num_to_audio_idx(level,ptr,sizeof(group)/sizeof(group[0]) -1);
			if (ret>0)
			{
				ptr +=ret;
				tts_play_set_group(group,(ptr-group),1,1);
			}
		}
	}
}
#endif

int FuncInputChk(void)
{
#if 1
    static uint32_t timer_show_free = 0;
	static uint32_t maxfree = 0,minfree = -1;
    unsigned int heap = 0;

    if( ql_rtos_get_systicks_to_ms() > timer_show_free )
    {
        timer_show_free = ql_rtos_get_systicks_to_ms() + 3000;

        heap = ql_rtos_get_free_heap_size();
        if( heap > maxfree )
        {
            maxfree = heap;
        }
        if( heap < minfree )
        {
            minfree = heap;
        }
        LOG_INFO("<memory> free %d, min free %d, max free %d\n", heap,minfree,maxfree);
    }
#endif

	#if 0
	AudioPlayLevel();
	#endif
	TermInfo.BatRemain = battery_read_percent();
	TermInfo.Charge = get_charge_status();
	if (TermInfo.Charge != TermInfo.ChargeOld)
	{
		LOG_INFO("enter %s , line %d, charge_status = %d,TermInfo.Charge %d\n", __func__, __LINE__, get_charge_status(), TermInfo.Charge);
		disp_update_request();
		TermInfo.ChargeOld = TermInfo.Charge;
		TermInfo.ChargeFull = 0;
		if (TermInfo.Charge)
		{
			TermLedShow(TERM_CHARGE_START);
			tts_play_set_idx(AUD_ID_CHARGE_IN,0,0);
			ql_rtos_task_sleep_ms(3000);// ��ƿ���3s
		}
		else
		{
			tts_play_set_idx(AUD_ID_CHARGE_OUT,0,0);
		}
			
		if (TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED)
			TermLedShow(TERM_NET_ABNORMAL);
		else 
			TermLedShow(TERM_NET_CON);
	}
	else if (TermInfo.Charge)
	{
		if(is_charge_full()!=TermInfo.ChargeFull)
		{
			TermInfo.ChargeFull=is_charge_full();
			if( TermInfo.ChargeFull )
			{
				TermLedShow(TERM_CHARGE_FULL);
			}
		}
	}
	else if (TermInfo.LowBat)
	{
		//LOG_INFO("enter %s , line %d,TermInfo.LowBat %d\n", __func__, __LINE__,TermInfo.LowBat);
		if (TermInfo.LowBat==1)
		{
			TermLedShow(TERM_LOWBAT);
			while((TermInfo.LowBat==1)&&(!TermInfo.Charge))
			{
				tts_play_set_idx(AUD_ID_PWR_LOWBAT,0,0);
				for(int ii=0;ii<30;ii++)
				{
					TermInfo.Charge = get_charge_status();
					if ((TermInfo.LowBat!=1)||(TermInfo.Charge==1)) 
						break;
					ql_rtos_task_sleep_ms(1000);
				}
				TermInfo.Charge = get_charge_status();
			}
		}
		if ((TermInfo.LowBat==2)&&(!TermInfo.Charge))
		{
			// battery is too low need auto powerdown
			for(int jj=0;jj<50;jj++)
			{
				if (tts_get_status() == 1)
				{
					break;
				}
				ql_rtos_task_sleep_ms(100);
			}
			disp_poweroff_msg();
			LOG_INFO("%s_%d ===battery is too low need auto powerdown\n", __func__, __LINE__);
			tts_play_set_idx(AUD_ID_PWR_LOWBATOFF,0,1);
			audio_clear_all_msg();
			ql_rtos_task_sleep_ms(3000);
			audio_clear_all_msg();
			AudioPlayHalt();
			TermInfo.Repeat = NULL;
			MqttExit();
            
			ql_power_down(0);
		}
	}
	return 0;
}

void NetChanlLTE(void)
{
	int i=0;
	int state = NET_DEVICE_STATE_DISCONNECTED;

	disp_gprs_mode();
	disp_set_net_connect_error(0);
	lpm_set(LPM_LOCK_NETWORK,1);

	if (sysparam_get_device_type() == 1)
	{
		tts_play_set_idx(AUD_ID_NET_MODE_GPRS,0,0);

		Ext_wifi_Off();		
    	ql_dev_set_modem_fun(QL_DEV_MODEM_FULL_FUN,0);
	}
	
	TermInfo.NetStatBak = NET_DEVICE_STATE_DISCONNECTED;
	TermInfo.PlayNetSucc = 0;

	disp_net_connecting();
	for(i=0; i<200; i++)
	{
		if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) 
			return;

		if (ModelInitResult())
		{
			break;
		}
		FuncInputChk();
		ql_rtos_task_sleep_ms(100);
	}

	switch(TermInfo.SIMState)
	{
		case 0x00:
		{
            short group[4];
            int cnt=0;
            TermLedShow(TERM_NO_SIM);
            disp_set_net_connect_error(1);
            group[cnt++]=AUD_ID_SIM_NOT_FOUND;
            if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
                 || ( sysparam_get( )->NetChanlLTE == WIFI_BAKE_MODE )
                 || ( sysparam_get( )->NetChanlLTE == WIFI_MODE ) )
			{
					group[cnt++]=AUD_ID_OR;
					group[cnt++]=AUD_ID_HOW_GPRS_TO_WIFI;
			}
            tts_play_set_group(group,cnt,0,0);
            uint32_t timer_notify = ql_rtos_get_systicks_to_ms() + 30 * 1000;
			while(1)
			{
				ql_rtos_task_sleep_ms(1000);
				if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) 
					break;
				FuncInputChk();
				if( ql_rtos_get_systicks_to_ms() > timer_notify )
				{
				    timer_notify = ql_rtos_get_systicks_to_ms() + 60 * 1000;
				    tts_play_set_idx(AUD_ID_SIM_NOT_FOUND,0,0);
				}
			}
		}	
			break;
		case 0x01:
		{
				short group[4];
				int cnt=0;
				
				TermLedShow(TERM_NET_START);
				group[cnt++]=AUD_ID_NET_CONNECTING;
				group[cnt++]=AUD_ID_PLEASE_WAITING;
				if ( ( sysparam_get( )->NetChanlLTE == Mode_NULL )
					|| ( sysparam_get( )->NetChanlLTE == WIFI_BAKE_MODE )
					|| ( sysparam_get( )->NetChanlLTE == WIFI_MODE ) )
				{
					group[cnt++]=AUD_ID_OR;
					group[cnt++]=AUD_ID_HOW_GPRS_TO_WIFI;
				}
				tts_play_set_group(group,cnt,0,0);
		}
			break;
		default:
			break; 
	}

	TermInfo.Repeat=FuncModuleErrRepeat;
	
	LOG_INFO("device_SN: %s\n", sysparam_get()->device_SN);
#ifdef ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
	if ((strlen(sysparam_get()->device_SN) == 0) || (strlen(sysparam_get()->device_name) == 0) ||(strlen(sysparam_get()->device_secret) == 0) || (strlen(sysparam_get()->product_key) == 0))
#else
	if (strlen(sysparam_get()->device_SN) == 0)
#endif
	{
	    disp_set_service_connect_error(1);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
	}
	else 
	{
		TermLedShow(TERM_ABNORMAL);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+60;
	}
    
	LOG_INFO("%s: loop start\n",__func__);
	while(1)
	{
		ql_rtos_task_sleep_ms(500);
		if((TermInfo.NetMode != GPRS_MODE)&&(TermInfo.NetMode != GPRS_BAKE_MODE)) break;
		FuncInputChk();
		if (TermInfo.Repeat!=NULL)
		{
			if (ql_rtos_get_systicks_to_s()>TermInfo.RepTim)
				TermInfo.Repeat();
		}

		if(TermInfo.PlayRecordMode)
		{
			Play_Record_Chk();
		}

		state = Module_Net_State();

		//network status changed
		if (state==TermInfo.NetStatBak)  continue;
	
		if(state)
		{
			if ((sysparam_get()->NetChanlLTE == WIFI_MODE) || (sysparam_get()->NetChanlLTE == WIFI_BAKE_MODE) )
			{
				TermInfo.NetMode = GPRS_MODE;
				sysparam_get()->NetChanlLTE = GPRS_MODE;
				sysparam_save();
			}
			else if(sysparam_get()->NetChanlLTE == Mode_NULL)
			{
				TermInfo.NetMode = GPRS_BAKE_MODE;
				sysparam_get()->NetChanlLTE = GPRS_BAKE_MODE;	
				sysparam_save();
			}
		}
			
		TermInfo.NetStatBak=state;
		if (state!=NET_DEVICE_STATE_CONNECTED) 
		{
			TermInfo.FailTime=ql_rtos_get_systicks_to_s();
			LOG_INFO("net fail time %d\n",TermInfo.FailTime);
			int module_state = NET_DEVICE_STATE_DISCONNECTED;
			lpm_set(LPM_LOCK_NETWORK,1);
			ql_rtos_task_sleep_ms(20000);
			module_state = Module_Net_State();
			if (module_state != NET_DEVICE_STATE_CONNECTED) 
			{
				TermInfo.NetStatBak=module_state;
				TermLedShow(TERM_NET_DIS);
				disp_set_net_connect_error(1);
				disp_sleep_enable( 0 );
				TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
				TermInfo.Repeat=FuncModuleErrRepeat;
				FuncModuleErrRepeat();
				continue;				
			}
			else
				continue;
		}
		lpm_set(LPM_LOCK_NETWORK,0);
		TermLedShow(TERM_NET_CON);
		if (TermInfo.PlayNetSucc==0)
		{
			TermInfo.PlayNetSucc=1;
			tts_play_set_idx(AUD_ID_NET_CONNECT_SUCESS,0,0);
		}
		mqtt_first_connet = 0;

		if( TermInfo.ServiceOnline )
		{
			disp_onoff_request( 1, DISP_HOLDON_MS );
			ql_rtos_task_sleep_ms( 50 );
			disp_set_service_connect_error( 0 );
			disp_sleep_enable( 1 );
		}

		TermInfo.Repeat=FuncLteRetryRepeat;
		FuncLteRetryRepeat();
	}
	TermInfo.Repeat=NULL;
	FuncInputChk();

	MqttExit();
}

void NetChanlWifi(void)
{
	disp_wifi_mode();
	disp_set_net_connect_error(0);
	lpm_set(LPM_LOCK_WIFI,1);
	lpm_set(LPM_LOCK_NETWORK,1);

	int state=NET_DEVICE_STATE_DISCONNECTED;
	int state_wifi=NET_DEVICE_STATE_DISCONNECTED;

	TermInfo.buttonAP = 0;
	Ext_wifi_Off();
	TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
	TermInfo.PlayNetSucc = 0;

	if(sysparam_get()->NetChanlLTE != Mode_NULL)
		tts_play_set_idx(AUD_ID_NET_MODE_WIFI,0,0);

    if (poweron_switch_modem_min_fun == 0)
    {
        ql_rtos_task_sleep_ms(1000);
        AudioPlayHalt();
    	ql_dev_set_modem_fun(QL_DEV_MODEM_MIN_FUN,0);
        AudioPlayContinue();
    }
    
	TermInfo.Repeat=FuncWifiErrRepeat;

	Ext_wifi_Open();		
	if (strlen(sysparam_get()->device_SN) == 0)
	{
		disp_set_net_connect_error(1);
		tts_play_set_idx(AUD_ID_PARAM_EMPTY,0,0);
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
	}
	else if (((sysparam_get()->wlan_sta_param.ssid_len) <= 0))
	{
		TermLedShow(TERM_ABNORMAL);
		disp_net_connecting();
		if(sysparam_get()->NetChanlLTE == Mode_NULL)
		{
			TermInfo.buttonAP = 2;
		}
		else
		{
			short group[4];
			int cnt=0;

			group[cnt++]=AUD_ID_HOW_WIFI_TO_AP;
			group[cnt++]=AUD_ID_OR;
			group[cnt++]=AUD_ID_HOW_WIFI_TO_GPRS;
			tts_play_set_group(group,cnt,0,0);
			
			TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
		}
	}
	else 
	{
	    disp_net_connecting();
		if ((sysparam_get()->wlan_sta_param.ssid_len) > 0)
		{
			if (wlan_sta_set((uint8_t *)sysparam_get()->wlan_sta_param.ssid, (uint8_t *)sysparam_get()->wlan_sta_param.psk)==0)
			{
				ql_rtos_task_sleep_ms(500);
				WifiSetOK=1;
				Ext_wifi_reBoot();
				LOG_INFO("%s,%d: -------------\n", __func__,__LINE__);
			}
			else
			{
			    disp_set_net_connect_error(1);
			}
			
//			wlan_sta_enable();
			LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
		}
		else
		{
			state_wifi =0;
			if (Wifi_Que_stat(&state_wifi)==0)
			{
				NetStat = ( state_wifi & 0x10 ) ? 1 : 0;
				WifiSetOK = ( state_wifi & 0x10 ) ? 1 : 0;
			}
		}
		TermInfo.RepTim=ql_rtos_get_systicks_to_s()+60;
	}

	LOG_INFO("%s: loop start,wifi ver=%s\n", __func__,sysparam_get()->WifiVer);
	while(1)
	{
		ql_rtos_task_sleep_ms(500);
		if((TermInfo.NetMode != WIFI_MODE)&&(TermInfo.NetMode != WIFI_BAKE_MODE)) break;
		
		if (TermInfo.buttonAP)
		{
			TermLedShow(TERM_NET_ABNORMAL);
			disp_net_connecting();
			MqttExit();
			NetStat = NET_DEVICE_STATE_DISCONNECTED;
			TermInfo.NetStatBak = NET_DEVICE_STATE_DISCONNECTED;
			
			if(WifiConfigSet() == 0)
			{
				LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
				WifiSetOK=1;
				TermInfo.PlayNetSucc = 0;
				NetStat = NET_DEVICE_STATE_CONNECTED;
				Ext_wifi_reBoot();
			}
			else
			{
				LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
				WifiSetOK=0;
			}
			TermInfo.buttonAP = 0;
			TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
			TermInfo.Repeat=FuncWifiErrRepeat;
			TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
			
		}
		if (Ext_wifi_reBoot()||(WifiSetOK==0))
		{
			WifiSetOK=0;
			NetStat=NET_DEVICE_STATE_DISCONNECTED;
			TermInfo.NetStatBak=NET_DEVICE_STATE_DISCONNECTED;
			if ((sysparam_get()->wlan_sta_param.ssid_len) > 0)
			{
				if (wlan_sta_set((uint8_t *)sysparam_get()->wlan_sta_param.ssid, (uint8_t *)sysparam_get()->wlan_sta_param.psk)==0)
				{
					ql_rtos_task_sleep_ms(500);
					WifiSetOK=1;
				}	
	//			wlan_sta_enable();
				LOG_INFO("enter %s , line %d,%s,%s\n", __func__, __LINE__,sysparam_get()->wlan_sta_param.ssid,sysparam_get()->wlan_sta_param.psk);
			}
			else
			{
				state_wifi =0;
				if (Wifi_Que_stat(&state_wifi)==0)
				{
					NetStat = ( state_wifi & 0x10 ) ? 1 : 0;
					WifiSetOK = ( state_wifi & 0x10 ) ? 1 : 0;
				}
			}
		}
		
		FuncInputChk();
		if (TermInfo.Repeat!=NULL)
		{
			if (ql_rtos_get_systicks_to_s()>TermInfo.RepTim)
				TermInfo.Repeat();
		}
		if(TermInfo.PlayRecordMode)
		{
			Play_Record_Chk();
		}

		state=NetStat;
		//����״̬�б仯
		if (state==TermInfo.NetStatBak)  continue;
		if(state)
		{
			if( (sysparam_get()->NetChanlLTE == GPRS_MODE)||(sysparam_get()->NetChanlLTE == GPRS_BAKE_MODE) )
			{
				TermInfo.NetMode = WIFI_MODE;
				sysparam_get()->NetChanlLTE = WIFI_MODE;
				sysparam_save();
			}
			else if(sysparam_get()->NetChanlLTE == Mode_NULL)
			{
				TermInfo.NetMode = WIFI_BAKE_MODE;
				sysparam_get()->NetChanlLTE = WIFI_BAKE_MODE;	
				sysparam_save();
			}
		}

		if (state!=NET_DEVICE_STATE_CONNECTED) 
		{
			TermInfo.FailTime=ql_rtos_get_systicks_to_s();
			LOG_INFO("net fail time %d\n",TermInfo.FailTime);
			lpm_set(LPM_LOCK_NETWORK,1);
			ql_rtos_task_sleep_ms(10000);
			state=NetStat;
			if (state != NET_DEVICE_STATE_CONNECTED) 
			{
				TermInfo.NetStatBak = state;
				TermLedShow( TERM_NET_DIS );
				disp_set_net_connect_error( 1 );
				disp_sleep_enable( 0 );
				TermInfo.RepTim=ql_rtos_get_systicks_to_s()+30;
				TermInfo.Repeat=FuncWifiErrRepeat;
				FuncWifiErrRepeat();
				continue;				
			}
			else
				continue;
		}
		Ext_wifi_netconnect_serial_921600();
		TermInfo.NetStatBak=state;
		lpm_set(LPM_LOCK_NETWORK,0);
		TermLedShow(TERM_NET_CON);
		if (TermInfo.PlayNetSucc==0)
		{
			TermInfo.PlayNetSucc=1;
			tts_play_set_idx(AUD_ID_NET_CONNECT_SUCESS,0,0);
		}
		mqtt_first_connet = 0;

		if( TermInfo.ServiceOnline )
		{
			disp_onoff_request( 1, DISP_HOLDON_MS );
			ql_rtos_task_sleep_ms( 50 );
			disp_set_service_connect_error( 0 );
			disp_sleep_enable( 1 );
		}
		TermInfo.Repeat=FuncWifiRetryRepeat;
		FuncWifiRetryRepeat();
	} 

    if (poweron_switch_modem_min_fun == 1)
    {
        LTE_CAT1_init();
    }
    poweron_switch_modem_min_fun = 0;

	TermInfo.Repeat=NULL;
	MqttExit();

	lpm_set(LPM_LOCK_WIFI,0);
}

void NetChanlNull(void)
{
    int i=0;
    disp_none_mode();

    for(i=0; i<20; i++)
    {
        if (ModelInitResult())
            break;
        else
            ql_rtos_task_sleep_ms(1000);
    }

    TermInfo.NetMode = GPRS_BAKE_MODE;
}

void list_dir(const char *path)
{
	QDIR * dp = NULL;
	struct lfs_info info = {0};
	int ret,sizeee;
	
	dp = ql_opendir(path);
	if(dp == NULL)
	{
		printf("[FS] *** dir open fail: %s ***\r\n", path);
		return;
	}
	usb_log_printf("[FS] dir opened: %s\r\n", path);

	usb_log_printf("[FS] ----- start dir list -----\r\n");
	usb_log_printf("[FS] type\tsize\tname\r\n");
	while(1)
	{
		ret = ql_readdir(dp, &info);
		if(ret < 0)
		{
			usb_log_printf("[FS] *** dir read fail: %s ***\r\n", path);
			break;
		}
		else if(ret == 0)
		{
			usb_log_printf("[FS] ----- end of dir list -----\r\n");
			break;
		}
		else
		{
			sizeee += info.size;
			usb_log_printf("[FS] %-4d\t%-4d\t%s\r\n", info.type, info.size, info.name);
		}
	}

	if(dp)
	{
		ql_closedir(dp);
		usb_log_printf("[FS] dir closed: %s,%d\r\n", path,sizeee);
	}
}

void App_task(void *pvParameters)
{
	start();
	{
		uint32_t disk_empty_size = ql_fs_free_size(U_DISK_SYM);
	    int disk_total_size = ql_fs_size(U_DISK_SYM);
	    usb_log_printf("%s_%d :U %d\r\n", __func__, __LINE__, disk_empty_size);
	    usb_log_printf("%s_%d :U  %d\r\n", __func__, __LINE__, disk_total_size);
		usb_log_printf("\r\n**********\r\n");
		disk_empty_size = ql_fs_free_size(B_DISK_SYM);
	    disk_total_size = ql_fs_size(B_DISK_SYM);
	    usb_log_printf("%s_%d :B  %d\r\n", __func__, __LINE__, disk_empty_size);
	    usb_log_printf("%s_%d :B  %d\r\n", __func__, __LINE__, disk_total_size);
		usb_log_printf("\r\n**********");
		list_dir(VOICE_PATH_PREFIX);
	}
	const dev_config_t *pdevconf = get_device_config();
	usb_log_printf("\n");
	usb_log_printf("======================================================================================\n\n");
	{
	char model_name[20]={0};
	ql_dev_get_model(model_name, 20);
	usb_log_printf(" device: %s-%s, hw: %s, sdk: %s, fw: %s, build: %s %s\n\n",
			pdevconf->devname,
			sysparam_get_device_type() == 0 ? "4G":"4GW",
			model_name,g_kernel_version,firmware_version + strlen(pdevconf->devname)+ strlen(g_kernel_version) + 2,
			__DATE__, __TIME__);
	}
	usb_log_printf("======================================================================================\n\n");

    if (sysparam_get_device_type() == 1)
	{
		TermInfo.NetMode = sysparam_get()->NetChanlLTE;
	}
	else
	{
		TermInfo.NetMode = GPRS_BAKE_MODE;
		if (poweron_switch_modem_min_fun == 1)
		{
			LTE_CAT1_init();
		}
		poweron_switch_modem_min_fun = 0;
	}

	while(1)
	{
		switch(TermInfo.NetMode)
		{
			case GPRS_MODE:
			case GPRS_BAKE_MODE:
			{
				LOG_INFO("<--NetChanlLTE-->\n");
				sockets_funcset(1);
				NetChanlLTE();
			}
			break;
			case WIFI_MODE:
			case WIFI_BAKE_MODE:
			{
				LOG_INFO("<--NetChanlWifi-->\n");
				sockets_funcset(0);
				NetChanlWifi();
			}
			break;
			case Mode_NULL:
			{
				LOG_INFO("<--Net Mode_NULL-->\n");
				NetChanlNull();
			}
			break;
			default:
				break;
		}
	}
}

void AppFsInit(void)
{
	qfs_init('S', "ymzn_param",0);
	if(get_device_config()->id==ID_DS10M)
	{
		ql_mkdir(CUST_PARAM_FILE_PATH_DS10M, 0x777);
		set_cust_param_path(CUST_PARAM_FILE_PATH_DS10M);
	}
	else if(get_device_config()->id==ID_DS10AK)
	{
		ql_mkdir(CUST_PARAM_FILE_PATH_DS10AK, 0x777);	
		set_cust_param_path(CUST_PARAM_FILE_PATH_DS10AK);
	}
}

void AppTaskInit(void)
{
	if (ql_rtos_task_create(&app_task_ctrl_thread,
						APP_TASK_CTRL_THREAD_STACK_SIZE,
						100,
						"App_task",
						App_task,
						NULL) != OS_OK) {
		LOG_INFO("--thread create error\n");
	}
}

void AppTaskDeInit(void)
{
	ql_rtos_task_delete(app_task_ctrl_thread);
	LOG_INFO("app_task_ctrl_thread delete\n");
}

void app_generate_kernel_app_version(void)
{
    char temp_buff[40];
    
    memset(temp_buff, 0, sizeof(temp_buff));
    ql_api_map_caller(QL_API_MAP_GET_KERNEL_VERSION, temp_buff);

    if (strlen(temp_buff) > 0)
    {
        memset(g_kernel_version, 0, sizeof(g_kernel_version));
        strcpy(g_kernel_version, temp_buff);

        memset(temp_buff, 0, sizeof(temp_buff));
        sprintf(temp_buff, "%s_%s_%s", get_device_config()->devname,g_kernel_version, firmware_version);
        memset(firmware_version, 0, sizeof(firmware_version));
        strcpy(firmware_version, temp_buff);

        memset(temp_buff,0,sizeof(temp_buff));
        ql_api_map_caller(QL_API_MAP_GET_KERNEL_BUILD_TIME, temp_buff);
        LOG_INFO("firmware info: app: %s, kernel: %s, build: %s\n",firmware_version,g_kernel_version,temp_buff);
    }
}
void ex_fs_init(void)
{
	int ret;
	int port_index = EXTERNAL_NORFLASH_PORT33_36_SSP0;
	int clk = _APBC_SSP_FNCLKSEL_1_625MHZ_;
	LOG_INFO("[FS] ========== exflash init  \r\n");
	ql_spi_nor_init(port_index, clk);	

	ret = qextfs_init('B', "customer_backup_fs", 0, port_index, 0, 0x800000);
	LOG_INFO("[FS] ========== exfs init : %d  \r\n", ret);
}


void evb_audio_main(void * param)
{
//	power_eventCheck(2000);
    mbedtls_platform_setup(NULL);
    memset(&TermInfo,0,sizeof(TermInfo));
    load_device_config(CONF_DEVTYPE_DEFAULT);
	AppFsInit();
	if(get_device_config()->id==ID_DS10M)
	{
		ex_fs_init();
	}
    app_generate_kernel_app_version();
    sysparam_init();
    ql_pwrkey_intc_enable(0);
    disp_port_init();

	isEnterFactorymode();

	if(sysparam_get()->uart_log == 0)
	{
		ql_debug_log_disable();
	}
    else
    {
        ql_debug_log_enable();
    }
	AppTaskInit();
//	return 0;
}

application_init(evb_audio_main, "evb_audio_main", 10, 1);
