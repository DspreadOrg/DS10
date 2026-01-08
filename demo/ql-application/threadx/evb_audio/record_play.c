#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_rtc.h"

#include "prj_common.h"
#include "cjson.h"
#include "record_play.h"
#include "record_store.h"
#include "fixaudio.h"
#include "tts_yt_task.h"
#include "terminfodef.h"
#include "public_api_interface.h"
#include "disp_port.h"

#if CFG_DBG
#define LOG_DBG(fmt, arg...) //usb_log_printf("[RECORD DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[RECORD INFO]"fmt, ##arg)
#else
#define LOG_DBG(fmt, arg...) ((void)0)
#define LOG_INFO(fmt, arg...) ((void)0)
#endif
Play_Record_Struct play_history_record = {0,0};

void Enter_Play_Record_Mode(void)
{
	LOG_INFO("%s\n",__func__);
	play_history_record.timeout_count = ql_rtos_get_systicks_to_ms() + 30 * 1000;
	TermInfo.PlayRecordMode = 1;
}

void Exit_Play_Record_Mode(void)
{
	LOG_INFO("%s\n",__func__);
	TermInfo.PlayRecordMode = 0;
	play_history_record.play_record_count = 0;
	play_history_record.timeout_count = 0;
	tts_play_set_idx(AUD_ID_EXIT_RECODE,0,1);
}

void Play_Record_Chk(void)
{
	if( ql_rtos_get_systicks_to_ms() > play_history_record.timeout_count )
	{
		Exit_Play_Record_Mode();	
	}
}

int display_record_func(uint16_t count) 
{
    cJSON*root = NULL;
	cJSON*psub = NULL;
	uint8_t play_len = 0;
	char play_buf[200] = {0};
	int date[5] = {0};
	uint32_t timestamp;
	struct tm *tm_ptr;
    int64_t money = 0;
    char *msg = NULL;
    int total_records = Record_Get_Count();
    
    Record_Manage_Read(count, play_buf, &play_len);
	LOG_INFO("%s: read %d/%d play_len:%d\n", __func__, count,total_records, play_len);
	if(play_len>0)
	{
		root=cJSON_Parse((void *)play_buf);
		psub = cJSON_GetObjectItem(root,"TimeStamp");
		timestamp = (uint32_t)psub->valueint+8*3600;
		//tm_ptr = ql_localtime(&timestamp);
		tm_ptr = gmtime( &timestamp );
		date[0] = tm_ptr->tm_mon;
		date[1] = tm_ptr->tm_mday;
		date[2] = tm_ptr->tm_hour;
		date[3] = tm_ptr->tm_min;
		date[4] = tm_ptr->tm_sec;
		LOG_INFO("%s ,line %d, time: %d-%d %d:%d:%d\n", __func__, __LINE__, date[0], date[1], date[2], date[3], date[4]);

		psub=cJSON_GetObjectItem(root,"Money");
		money = psub->valueint;

		psub=cJSON_GetObjectItem(root,"VoiceMsg");	

		if ((TermInfo.disp.tm1604)||(TermInfo.disp.fg00ahk))
		{
			if (( count >= total_records )||(count >= RECORD_PAY_BACKUP_CNT))
			{
				tts_play_set_idx(AUD_ID_RECODE_LAST,1,1);
			}
			else if( count <= 1 )
			{
				tts_play_set_idx(AUD_ID_RECODE_FIRST,1,1);
			}
			else
			{
				tts_play_set_idx(AUD_ID_KEY_BEEP,1,1);
			}
			if( money )
			{
				disp_set_paymsg( money, timestamp-8*3600, TTS_FLAG_RECORD );
				disp_onoff_request( 1, DISP_HOLDON_MS );
	 		}
		}
		else
		{
			if ( count >= total_records )
			{
				count |=0x80;
			}
			tts_play_set_record_idx(count,psub->valuestring, money, timestamp);
		}
		cJSON_Delete(root);
	}
	else
	{
		usb_log_printf("enter %s ,line %d, no play record\n", __func__, __LINE__);
		tts_play_set_idx(AUD_ID_RECODE_NOT_FOUND,1,1);
	}
	if( msg )
	{
	    free(msg);
	}
	return 0;
}

