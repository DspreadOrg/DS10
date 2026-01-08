#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_audio.h"
//#include "ql_flash.h"
#include "ql_power.h"
#include "ql_nw.h"
#include "prj_common.h"
#include "tts_yt_task.h"
#include "led.h"
#include "fixaudio.h"
#include "gpio.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "button.h"
#include "module_lte.h"
#include "drv_bat.h"
#include "record_store.h"
#include "record_play.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "public_api_interface.h"

#define LOG_DBG(fmt, arg...) //usb_log_printf("[DBG BUTTON]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[INFO BUTTON]"fmt, ##arg)

#define SYSPARAM_DATA_SAVE_TIME_GAP      5 // sysparam will save after x seconds when the data is change
#define ExitPlayRecModeTimeSec	18
 
GPIO_Button_Cmd_Info Gpio_Button_Cmd = {GPIO_BUTTON_CMD_NULL, GPIO_BUTTON_NUM};
PLAYER_CMD read_payer_ctrl_cmd();

//ql_queue_t buttonQueue = NULL;
ql_task_t button_queue_task_thread = NULL;

ql_queue_t keyValueQueue = NULL;
ql_task_t key_value_queue_task_thread = NULL;

#define ModePlayRecord	0
#define ModeVolSet	1
struct {
	char Mode;
	uint32_t Sec;
}playRecordMod=
{
	ModePlayRecord,0
};
//static uint32_t playRecordMod = 0;
static uint8_t playRecordIndex = 0;

extern void device_refactory(void);
extern int MqttExit(void);


const Gpio_Button_Cfg button3_cmd_list[GPIO_BUTTON_NUM]=
{
	//GPIO_BUTTON_CMD_LONG_PRESS GPIO_BUTTON_CMD_SHORT_PRESS GPIO_BUTTON_CMD_DOUBLE_CLICK GPIO_BUTTON_CMD_REPEAT GPIO_BUTTON_CMD_RELEASE
	{GPIO_BUTTON_0,{	CMD_PLAYER_POWEROFF,	CMD_PLAYER_EXIT_AP,			CMD_PLAYER_MODE_FUNC,	CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_1,{	CMD_PLAYER_NULL,  		CMD_PLAYER_VOLUME_ROLL,	CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_2,{	CMD_PLAYER_SEL_CHANL,	CMD_PLAYER_TRANS_RECORD,		CMD_PLAYER_AP_MODE,				CMD_PLAYER_NULL,		CMD_PLAYER_NULL}}, 
	{GPIO_BUTTON_3,{	CMD_PLAYER_REFACTORY,	CMD_PLAYER_NULL,   			CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_4,{	CMD_PLAYER_RECORD,		CMD_PLAYER_NULL,   			CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
};

const Gpio_Button_Cfg button4_cmd_list[GPIO_BUTTON_NUM]=
{
	//GPIO_BUTTON_CMD_LONG_PRESS GPIO_BUTTON_CMD_SHORT_PRESS GPIO_BUTTON_CMD_DOUBLE_CLICK GPIO_BUTTON_CMD_REPEAT GPIO_BUTTON_CMD_RELEASE
	{GPIO_BUTTON_0,{	CMD_PLAYER_POWEROFF,		CMD_PLAYER_EXIT_AP,						CMD_PLAYER_MODE_FUNC,			CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_1,{	CMD_PLAYER_VOLUME_DOWN,	CMD_PLAYER_VOLUME_DOWN,				CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_2,{	CMD_PLAYER_SEL_CHANL,		CMD_PLAYER_SWITCH_QUERY_MODE,	CMD_PLAYER_AP_MODE,				CMD_PLAYER_NULL,		CMD_PLAYER_NULL}}, 
	{GPIO_BUTTON_3,{	CMD_PLAYER_VOLUME_UP,		CMD_PLAYER_VOLUME_UP,					CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_4,{	CMD_PLAYER_REFACTORY,		CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_5,{	CMD_PLAYER_RECORD,			CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
	{GPIO_BUTTON_6,{	CMD_PLAYER_NULL,				CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
//	{GPIO_BUTTON_7,{	CMD_PLAYER_NULL,				CMD_PLAYER_NULL,							CMD_PLAYER_NULL,					CMD_PLAYER_NULL,		CMD_PLAYER_NULL}},
};

Gpio_Button_Cfg const * button_cmd_list=NULL;
		
void AtLogSet(int set)
{
	//set=0~2
	int ret = 0;
	char resp_buf[128] = {0};
	char atcmd[64];

	sprintf(atcmd,"at+log=18,%d\r\n",set);
	ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
	usb_log_printf("=============%s , line %d,%s,ret %d===========\n", __func__, __LINE__,atcmd,ret);
}

PLAYER_CMD read_payer_ctrl_cmd(GPIO_Button_Cmd_Info *btinfo)
{
	PLAYER_CMD cmd = CMD_PLAYER_NULL;
	const dev_config_t *pdevconf = get_device_config();
	Gpio_Button_Cmd = *btinfo;

	LOG_INFO("%s: id=%d, cmd=%d\n", __func__, btinfo->id,btinfo->cmd);

	if ((Gpio_Button_Cmd.cmd != GPIO_BUTTON_CMD_NULL)&&(button_cmd_list!=NULL))
	{
		cmd = button_cmd_list[Gpio_Button_Cmd.id].button_cmd_val[Gpio_Button_Cmd.cmd];;
		Gpio_Button_Cmd.cmd = GPIO_BUTTON_CMD_NULL;
		Gpio_Button_Cmd.id = GPIO_BUTTON_NUM;

		LOG_INFO("%s: id=%d, cmd=%d, func=%d\n",__func__,Gpio_Button_Cmd.id,Gpio_Button_Cmd.cmd,cmd);
	}

	// wakeup display
	disp_onoff_request(1,DISP_HOLDON_MS);

	return cmd;
}


void player_pwroff_func( char saveflag )
{
    LOG_INFO("---- buttons CMD_PLAYER_POWEROFF  \n");
	if(ql_rtos_get_systicks_to_s()>5)
	{
        disp_poweroff_msg();
        tts_play_set_idx(AUD_ID_PWR_OFF,0,1);
        audio_clear_all_msg();
        ql_rtos_task_sleep_ms(1000);
        audio_clear_all_msg();
        AudioPlayHalt();
        
        TermLedShow(TERM_POWEROFF);
        
        TermInfo.Repeat = NULL;
        MqttExit();

        if (saveflag == 1)
        {
            sysparam_save();
            ql_rtos_task_sleep_ms(200);
        }
        disp_onoff_request(0,0);
        ql_rtos_task_sleep_ms(500);
        ql_power_down(0);
	}
	else
	{
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
	}
}

void ExitPlayRecordModChk(void)
{
	if (playRecordMod.Sec<ql_rtos_get_systicks_to_s())
	{
		if (playRecordMod.Mode==ModePlayRecord)
			playRecordMod.Mode=ModeVolSet;
		disp_set_updown_state( DISP_HIDE_UPDOWN );
		disp_record_index( -1, DISP_HOLDON_FOREVER);
		playRecordIndex=0;
	}
}

void player_switch_query_mode_func(void)
{
    if ( ( TermInfo.LowBat ) && ( !get_charge_status( ) ) )
    {
        tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    }
    else
    {
    	playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
        if ( playRecordMod.Mode==ModeVolSet )
        {
            LOG_INFO( "%s: enter query mode\n", __func__ );
			playRecordMod.Mode=ModePlayRecord;
            playRecordIndex = 0;
            if ( 0 < Record_Get_Count( ) )
            {
                disp_set_updown_state( DISP_SHOW_UP );
            }
			tts_play_set_idx(AUD_ID_RECODE_MODE,1,1);

            disp_record_index( 0 , DISP_HOLDON_FOREVER);
        }
        else
        {
            LOG_INFO( "%s: exit query mode\n", __func__ );
            disp_set_updown_state( DISP_HIDE_UPDOWN );
            playRecordMod.Mode=ModeVolSet;
			tts_play_set_idx(AUD_ID_EXIT_RECODE,1,1);
            disp_record_index( -1, DISP_HOLDON_FOREVER);
        }
    }
}

void player_mode_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_MODE_FUNC\n");
	if((TermInfo.LowBat)&&(!get_charge_status()))
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		short group[10];
		short * ptr;
		int ret;
		
		ptr=group;
#if 0
		if(get_charge_status())
		{
			*ptr++=AUD_ID_CHARGE_IN;
		}
		else if (battery_read_percent()>=0)
		{
			*ptr++=AUD_ID_PWR_PER;
			ret=num_to_audio_idx(battery_read_percent()*100,ptr,sizeof(group)/sizeof(group[0]) -1);
			if (ret>0)
			{
				ptr +=ret;
			}
		}
#endif		
		//����
		if (sysparam_get_device_type() == 0)
		{
			if((TermInfo.NetStatBak!=NET_DEVICE_STATE_CONNECTED))
			{
				if(TermInfo.SIMState == 0x00)//δ��⵽SIM��
				{
					*ptr++=AUD_ID_SIM_NOT_FOUND;
				}
				else//�����쳣
				{
					*ptr++=AUD_ID_NET_NOT_CONNECT;
				}
			}
			else//��������
			{
				*ptr++=AUD_ID_NET_CONNECT;
			}
		}
		else
		{
			if((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE))
					*ptr++=AUD_ID_NET_MODE_GPRS;
			else
			{
				if(TermInfo.buttonAP == 0)
					*ptr++=AUD_ID_NET_MODE_WIFI;
				else if(TermInfo.buttonAP == 1)
					*ptr++=AUD_ID_NET_MODE_WIFI_AIRKISS;
				else if(TermInfo.buttonAP == 2)
					*ptr++=AUD_ID_NET_MODE_WIFI_AP;
			}	
		}
		tts_play_set_group(group,(ptr-group),1,1);
	}
}

void player_refactory_func(void)
{
	if((TermInfo.LowBat)&&(!get_charge_status()))
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		device_refactory();
		LOG_INFO("----player_task CMD_PLAYER_REFACTORY\n");
	}
}

void player_sel_chanl_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_SEL_CHANL\n");
	if (TermInfo.OTAMode)
	{
		return;
	}
				
	if((TermInfo.LowBat)&&(!get_charge_status()))
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);

		if (sysparam_get_device_type() == 1)
		{
			if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
			{
				TermInfo.NetMode = GPRS_MODE;
			}
			else if((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE))
			{
				TermInfo.NetMode = WIFI_MODE;
			}
			if ((TermInfo.buttonAP)&&((TermInfo.NetMode == GPRS_MODE)||(TermInfo.NetMode == GPRS_BAKE_MODE)))
			{
				if( (TermInfo.buttonAP == 1) || (TermInfo.buttonAP == 2) )
				{
					TermInfo.buttonAP=0;
				}
			}	
		}
	} 
}


void player_exit_ap_func(void)
{
	LOG_INFO("----player_task CMD_PLAYER_EXIT_AP\n");
	if((TermInfo.LowBat)&&(!get_charge_status()))
		//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);

	if (sysparam_get_device_type() == 0)
	{
			//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
	}
	else
	{
		if( (TermInfo.buttonAP == 1) || (TermInfo.buttonAP == 2) )
		{
			TermInfo.buttonAP=0;
			//tts_play_immediately(AudioYitcpw,AudioYitcpwLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_EXIT_WIFI_SET,1,1);
		}
		else
		{
			//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
		}
	}
}
    
void player_ap_mode_func(void)
{
    LOG_INFO("----player_task CMD_PLAYER_AP_MODE\n");
    if((TermInfo.LowBat)&&(!get_charge_status()))
    {
//        tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    }
    else
    {
        if (sysparam_get_device_type() == 0)
        {
//			tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
			tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
        }
        else if((TermInfo.NetMode == WIFI_MODE)||(TermInfo.NetMode == WIFI_BAKE_MODE))
        {
            if(TermInfo.buttonAP == 0)
            {
                LOG_INFO("----player_task AD button0 long press enter ap mode\n");
                TermInfo.buttonAP = 2;
            }
            else if(TermInfo.buttonAP == 2)
            {
                //Ext_NetConfig_exit();
                TermInfo.buttonAP=1;
            }
            else if(TermInfo.buttonAP == 1)
            {
                TermInfo.buttonAP=0;
				tts_play_set_idx(AUD_ID_EXIT_WIFI_SET,1,1);
                //Ext_NetConfig_exit();
            }
        }
    }
}

int player_volume_roll_func(void)
{
    static int *current_volume = NULL;
    static int firstFlag = 0;
    static char RollDir = 0;
    int ret = -1;
    
    LOG_INFO("----player_task CMD_PLAYER_VOLUME_ROLL\n");

    if (firstFlag == 0)
    {
        current_volume = &sysparam_get()->volume;
        if (*current_volume >= SPEADKERS_VOLUME_SET_MAX)
        {
            RollDir = 0;
        }
        else if (*current_volume <=1)
        {
            RollDir = 1;
        }
        firstFlag = 1;
    }
    
	if((TermInfo.LowBat)&&(!get_charge_status()))
//		tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
		tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
	else
	{
		if (RollDir)
		{
			*current_volume += 1;
			if (*current_volume >= SPEADKERS_VOLUME_SET_MAX) 
			{
				*current_volume = SPEADKERS_VOLUME_SET_MAX;
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				RollDir=0;
				//tts_play_immediately(AudioYinlzd,AudioYinlzdLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_VOL_MAX,1,1);
			}
			else
			{
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
		}
		else
		{
			*current_volume -= 1;
			if (*current_volume <= 1) 
			{
				*current_volume=1;
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
                
				RollDir=1;
				//tts_play_immediately(AudioYinlzx,AudioYinlzxLen,FixAudioTypeDef);
				tts_play_set_idx(AUD_ID_VOL_MIN,1,1);
			}
			else
			{
				AuidoVolumeSet(*current_volume);
				ql_rtos_task_sleep_ms(20);
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
		}
		LOG_DBG("current_volume %d\n", *current_volume);
		disp_vol_update(0);
        ret = 0;
	}
	
    return ret;
}

int player_volume_up_func(void)
{
    static int *current_volume = NULL;
    int recordPlayCnt = 0;
    int ret = -1;
    
    current_volume=&sysparam_get()->volume;
    
    if((TermInfo.LowBat)&&(!get_charge_status())) 
    {
      //tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
    	ExitPlayRecordModChk();
   		playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
    	
    	if (playRecordMod.Mode==ModePlayRecord)
//    	if (!(ql_rtos_get_systicks_to_s()<playRecordMod))
    	{
            //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            recordPlayCnt = Record_Get_Count();
            LOG_INFO("recordTotal = %d, current = %d\n", recordPlayCnt,playRecordIndex);
            if ( recordPlayCnt <= 0 ) {
                //ql_rtos_task_sleep_ms(80);
                //tts_play_set(AudioZanwskjl, AudioZanwskjlLen, GBK_TEXT);
                //tts_play_set(AudioZanwskjl,AudioZanwskjlLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
                disp_set_updown_state(DISP_HIDE_UPDOWN);
                return 0;
            }
            if (recordPlayCnt>RECORD_PAY_BACKUP_CNT) 
            	recordPlayCnt=RECORD_PAY_BACKUP_CNT;
            if ( playRecordIndex >= recordPlayCnt )
            {
                playRecordIndex = recordPlayCnt;
            }
            
            if( playRecordIndex > 1 )
            {
                playRecordIndex--;
            }

            if (playRecordIndex == 0 || playRecordIndex == 1) {
                playRecordIndex=1;
                disp_set_updown_state(DISP_SHOW_DOWN);
//                tts_play_immediately(AudioZjyb,AudioZjybLen,FixAudioTypeDef);
#if 0
                display_record_func(playRecordIndex);
                return 0;
#endif
            }
            else
            {
                disp_set_updown_state(DISP_SHOW_UPDOWN);
//                tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            }

            disp_record_index(playRecordIndex, DISP_HOLDON_FOREVER);
#if 1
            display_record_func(playRecordIndex);
#else
            tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
#endif
            
        } 
        else 
        {
        	*current_volume += 1;
        	if (*current_volume >= SPEADKERS_VOLUME_SET_MAX) 
        	{
        		*current_volume = SPEADKERS_VOLUME_SET_MAX;
        		AuidoVolumeSet(*current_volume);
        		ql_rtos_task_sleep_ms(20);
        		//tts_play_immediately(AudioYinlzd,AudioYinlzdLen,FixAudioTypeDef);
        		tts_play_set_idx(AUD_ID_VOL_MAX,1,1);
        	}
        	else
        	{
        		AuidoVolumeSet(*current_volume);
        		ql_rtos_task_sleep_ms(20);
        		//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
        		tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
        	}
        	LOG_DBG("current_volume %d\n", *current_volume);
            ret = 0;

            disp_vol_update(1);
        }

    }

    return ret;
}

int player_volume_down_func(void)
{
    static int *current_volume = NULL;
    int recordPlayCnt = 0;
    int ret = -1;
    
    current_volume = &sysparam_get()->volume;
    if ((TermInfo.LowBat)&&(!get_charge_status()))
    {
			//tts_play_immediately(AudioDianldqcd,AudioDianldqcdLen,FixAudioTypeDef);
			tts_play_set_idx(AUD_ID_PWR_LOWBAT,1,1);
    } 
    else 
    {
    	ExitPlayRecordModChk();
   		playRecordMod.Sec=ql_rtos_get_systicks_to_s()+ExitPlayRecModeTimeSec;
    	
    	if (playRecordMod.Mode==ModePlayRecord)
//    	if (!(ql_rtos_get_systicks_to_s()<playRecordMod))
    	{
            //tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            recordPlayCnt = Record_Get_Count();
            LOG_INFO("recordTotal = %d, current = %d\n", recordPlayCnt,playRecordIndex);

            if ( recordPlayCnt <= 0 ) {
                //tts_play_immediately(AudioZanwskjl,AudioZanwskjlLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
                disp_set_updown_state(DISP_HIDE_UPDOWN);
                return 0;
            }
            
            if (recordPlayCnt>RECORD_PAY_BACKUP_CNT) 
            	recordPlayCnt=RECORD_PAY_BACKUP_CNT;
            	
            if (playRecordIndex==0)
            	playRecordIndex=recordPlayCnt;
            else
            	playRecordIndex++;
            	
            if ( playRecordIndex >= recordPlayCnt ) {
//                tts_play_immediately(AudioZhyb,AudioZhybLen,FixAudioTypeDef);
                playRecordIndex = recordPlayCnt;
                disp_set_updown_state(DISP_SHOW_UP);
            }
            else
            {
                disp_set_updown_state(DISP_SHOW_UPDOWN);
//                tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
            }
            
            disp_record_index(playRecordIndex, DISP_HOLDON_FOREVER);
#if 1
            display_record_func(playRecordIndex);
#else
            tts_play_immediately((char *)&playRecordIndex,sizeof(playRecordIndex),RECORD_TEXT);
#endif
        } 
        else
        {
            *current_volume -= 1;
            if (*current_volume <= 1) 
            {
	            *current_volume=1;
	            AuidoVolumeSet(*current_volume);
	            ql_rtos_task_sleep_ms(20);
	            //tts_play_immediately(AudioYinlzx,AudioYinlzxLen,FixAudioTypeDef);
	            tts_play_set_idx(AUD_ID_VOL_MIN,1,1);
            }
            else
            {
	            AuidoVolumeSet(*current_volume);
	            ql_rtos_task_sleep_ms(20);
	            //tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
	            tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
            }
            LOG_DBG("current_volume %d\n", *current_volume);
            ret = 0;
            disp_vol_update(1);
        }    
    }

    return ret;
}

void player_record_func(void)
{
    LOG_INFO( "%s: call\n",__func__ );
//    tts_play_immediately(AudioAnjy,AudioAnjyLen,FixAudioTypeDef);
    //tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
    tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
    Enter_Play_Record_Mode( );
}
static int ota_sta=0;
int OTA_button_disable(void)
{
	return ota_sta;
}
void set_OTA_sta(int set)
{
	ota_sta=set;
}
void button_queue_event(void *pvParameters)
{
	char saveflag=0;
	uint8_t recv_net_msg = 0;
	int change_volume_time = 0;
	GPIO_Button_Cmd_Info button_cmd;
	static int y=0;
#if 1
	if( (getAddVolumeButtonGpio() <= GPIO_PIN_NO_NOT_ASSIGNED) || (getAddVolumeButtonGpio() >= GPIO_PIN_NO_MAX) )
		button_cmd_list=button3_cmd_list;	//����
	else
		button_cmd_list=button4_cmd_list;	//�ļ�
#endif
    
	while(1)
	{
		ExitPlayRecordModChk();

	    if (ql_rtos_queue_wait(keyValueQueue, (u8*)&button_cmd, sizeof(GPIO_Button_Cmd_Info), 3000) == OS_OK)
        {
	        recv_net_msg = read_payer_ctrl_cmd((GPIO_Button_Cmd_Info *)&button_cmd);
	        if( CMD_PLAYER_NULL == recv_net_msg )
	        {
	            continue;
	        }
        }
	    else
	    {
            if ( tts_get_status( ) != 2 )
            {
                if ( ( saveflag ) && ( ( ql_rtos_get_systicks_to_s( ) - change_volume_time )  >= SYSPARAM_DATA_SAVE_TIME_GAP ) )
                {
                    sysparam_save( );
                    saveflag = 0;
                }
            }
            continue;
	    }
		if(OTA_button_disable()&&(recv_net_msg!=CMD_PLAYER_POWEROFF))//OTA����ʱ����������������
			continue;

        LOG_INFO("%s__%d, %d\n", __func__, __LINE__, recv_net_msg);
		//current_volume=&sysparam_get()->volume;
		
		switch(recv_net_msg)
		{
			case CMD_PLAYER_NEXT:
				LOG_INFO("----player_task AD button0 \n");
				break;
            
            case CMD_PLAYER_VOLUME_UP:
				if ( !player_volume_up_func() )
				{
					saveflag = 1;
					change_volume_time = ql_rtos_get_systicks_to_s();
				}
                break;

            case CMD_PLAYER_VOLUME_DOWN:
				if ( !player_volume_down_func() )
				{
					saveflag = 1;
					change_volume_time = ql_rtos_get_systicks_to_s();
				}
				break;
            
			case CMD_PLAYER_VOLUME_ROLL:
				if ( !player_volume_roll_func() ) 
				{
					saveflag = 1;
					change_volume_time = ql_rtos_get_systicks_to_s();
				}
				break;
                
			case CMD_PLAYER_AP_MODE:
				player_ap_mode_func();
				break;
            
			case CMD_PLAYER_EXIT_AP:
				player_exit_ap_func();
				break;
            
		    case CMD_PLAYER_SEL_CHANL:
				player_sel_chanl_func();
				break;
            
			case CMD_PLAYER_REFACTORY:
				player_refactory_func();	
				break;
                
			case CMD_PLAYER_MODE_FUNC:
				player_mode_func();	
				break;
            
			case CMD_PLAYER_RECORD:
				LOG_INFO("----player_task CMD_PLAYER_RECORD\n");
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				Enter_Play_Record_Mode();
				break;
                
			case CMD_PLAYER_TRANS_RECORD:
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				//extern int latency2;
				////latency2++;
				//if (latency2<4) latency2++;
//			player_trans_record_func();
				break;
            
			case CMD_PLAYER_POWEROFF:
				player_pwroff_func(saveflag);
				break;

			case CMD_PLAYER_SWITCH_QUERY_MODE:
				player_switch_query_mode_func();

				break;

                
			default:
				//tts_play_immediately(AudioInputOK,AudioInputOKLen,MEM_PCM_TEXT);
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
				break;
		}
	}
}

QlOSStatus button_task_init()
{
	if(ql_rtos_queue_create(&keyValueQueue, sizeof(GPIO_Button_Cmd_Info), 1) != OS_OK) {
		LOG_INFO("ql_rtos_queue_create create KeyValQueue error\n");
		return -1;
	}
	
	if (ql_rtos_task_create(&button_queue_task_thread,
						25*1024,
						81,
						"button_queue_event",
						button_queue_event,
						NULL) != OS_OK) {
		LOG_INFO("button_queue_task_thread create error\n");
		return -1;
	}

	return 0;
}

