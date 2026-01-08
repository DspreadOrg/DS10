#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "ql_rtos.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "systemparam.h"
#include "tts_yt_task.h"
#include "terminfodef.h"
#include "public_api_interface.h"
#include "utf8_unicode.h"
#include "gpio.h"
#include "res.h"
#include "pcm_play_num.h"
#include "voice_wav_mix.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "utf8strdef.h"
#include "ql_spi_nor.h"

//#if TTS_SEL==TTS_YTKJ

#define TAG "[TTS]"
#define TTS_DIR "0:/tts"

#if CFG_DBG
#define LOG_DBG(fmt, arg...) //usb_log_printf("[TTS INFO]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[TTS INFO]"fmt, ##arg)
#else
#define LOG_DBG(fmt, arg...) ((void)0)
#define LOG_INFO(fmt, arg...) ((void)0)
#endif

#define AUDIO_WAIT_DELAY_TIMES        (20 * 1000)    // wait 20s max
#define AUDIO_MP3_MEMORY_BUF_LEN      (600)
#define AUDIO_MP3_MEMORY_PLAY_LEN     (256)

#define AUDIO_PLAY_NEED_SKIP_HEAD     0

#define WAIT_NETWORK_REGISTER_MAX_COUNT       65//55

#define PLAY_TYPE_KEY                 0x01
#define PLAY_TYPE_TTS                 0x02
#define PLAY_TYPE_MASK                (PLAY_TYPE_KEY | PLAY_TYPE_TTS)

#define TTS_SEPARATE_USE

//ql_task_t tts_play_task_ctrl_thread = NULL;
ql_task_t tts_decode_task_ctrl_thread = NULL;
ql_mutex_t xSemaphore = NULL;

ql_queue_t ttsPlayQueue = NULL;
ql_queue_t vKeyQueue = NULL;
static ql_flag_t play_request_flag = NULL;
QL_TTS_ENVENT_E tts_play_state = QL_TTS_EVT_PLAY_START;
static enum_aud_player_state audio_play_state = AUD_PLAYER_START;
static unsigned char audio_file_play_flag = 0;
unsigned char tts_play_status = 0;

void print_hex(const char *title, const unsigned char buf[], size_t len);
extern int get_factory_test_status(void);

extern unsigned char g_network_register_status;
extern bool audio_pause_play;
static void amrPlaybackEvent_cd(AmrPlaybackEventType event, int val)
{
//	LOG_INFO("%s: amr_CD--event: %d val = %d\n", __func__,event,val);
    if(event == AMR_PLAYBACK_EVENT_STATUS){
		if(val == AMR_PLAYBACK_STATUS_STARTED)
		{
			printf("amr start\r\n");
		}
		if(val == AMR_PLAYBACK_STATUS_ENDED)
		{
			audio_play_state = val;
			printf("amr end\r\n");        
		}
		/*
		if(val == AMR_STREAM_STATUS_NEARLY_OVERRUN)
		{
			audio_play_state = val;
			printf("amr overrun\r\n");        
		}
		if(val == AMR_STREAM_STATUS_NEARLY_UNDERRUN)
		{
			audio_play_state = val;
			printf("amr underrun\r\n");        
		}
		*/
    }
}


int amr_stream_play(unsigned char *amr_buff, int amr_size)
{
	int i = 0;
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	AmrPlaybackHandle AMRhandle;
	AUDIO_PLAY_OPTION prop = {0};
	prop.latency2 = 4;

	AmrPlaybackConfigInfo amr_configInfo = {
		prop.option,
		0,
		amrPlaybackEvent_cd
	};

	ql_play_amr(0,&AMRhandle,&amr_configInfo);
	audio_play_state = AMR_PLAYBACK_STATUS_STARTED;
	LOG_DBG("%s: start\n",__func__);
	ql_play_amr_stream_buffer(AMRhandle,amr_buff, amr_size);
	ql_stop_amr_play(AMRhandle,1);

	for (i = 0; i < delay_times; i++)
	{
		if ((audio_play_state == AMR_PLAYBACK_STATUS_ENDED))
		{
			break;
		}
		ql_rtos_task_sleep_ms(50);
	}
	LOG_DBG("%s: end\n",__func__);
	return 0;
}

int amr_file_play(char *file_name)
{
	AmrPlaybackHandle AMRhandle;
	AUDIO_PLAY_OPTION prop = {0};
	prop.latency2 = 4;

	AmrPlaybackConfigInfo amr_configInfo = {
		prop.option,
		0,
		amrPlaybackEvent_cd
	};

	ql_play_amr(file_name,&AMRhandle,&amr_configInfo);

	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	LOG_DBG("%s: start\n",__func__);
	audio_play_state = AMR_PLAYBACK_STATUS_STARTED;
	for (int i = 0; i < delay_times; i++)
	{
	
		if ((audio_play_state == AMR_PLAYBACK_STATUS_ENDED))
		{
			break;
		}
		ql_rtos_task_sleep_ms(50);
	}
	ql_stop_amr_play(AMRhandle,0);
	LOG_DBG("%s: end\n",__func__);
	return 0;
}

 static void Mp3PlaybackEvent_cd(Mp3PlayEventType event, int val)
{
	//LOG_INFO("%s: MP3_CD--event: %d val = %d\n", __func__,event,val);
    if(event == MP3_PLAYBACK_EVENT_STATUS){
	
       if(val == MP3_PLAYBACK_STATUS_STARTED)
       {
            printf("mp3 start\r\n");
       }
       if(val == MP3_PLAYBACK_STATUS_ENDED)
       {
			audio_play_state = val;
            printf("mp3 end\r\n");        
       }
	   /*
       if(val == MP3_STREAM_STATUS_NEARLY_OVERRUN)
       {
			audio_play_state = val;
            printf("mp3 overrun\r\n");        
       }
       if(val == MP3_STREAM_STATUS_NEARLY_UNDERRUN)
       {
			audio_play_state = val;
            printf("mp3 underrun\r\n");        
       }
	   */
    }
}


int mp3_stream_play(unsigned char *mp3_buff, int mp3_size)
{
	Mp3PlaybackHandle MP3handle;
	AUDIO_PLAY_OPTION prop = {0};
	prop.latency2 = 4;

	Mp3PlayConfigInfo mp3_configInfo = {
		prop.option,//x01ll<<46,
		0,
		0,
		0,
		Mp3PlaybackEvent_cd,
		0
	};

	LOG_INFO("%s: mp3_configInfo.option: 0x%016llX\n", __func__,mp3_configInfo.option);
	ql_play_mp3(0,&MP3handle,&mp3_configInfo);
	audio_play_state = MP3_PLAYBACK_STATUS_STARTED;
	LOG_DBG("%s: start\n",__func__);
	ql_play_mp3_stream_buffer(MP3handle,mp3_buff, mp3_size);
	ql_stop_mp3_play(MP3handle,1);
	
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	for (int i = 0; i < delay_times; i++)
	{
		if ((audio_play_state == MP3_PLAYBACK_STATUS_ENDED))
		{
			break;
		}
		ql_rtos_task_sleep_ms(50);
	}
	
	LOG_DBG("%s: play end\n",__func__);
}

#ifdef TTS_SEPARATE_USE
static unsigned int tts_address = 0;
int read_cb(void *pParameter, void * pBuffer, unsigned long iPos, unsigned long nSize)
{
	//ql_spi_nor_read_fifo(EXTERNAL_NORFLASH_PORT33_36, pBuffer, tts_address+iPos, nSize);
	ql_spi_nor_read_fifo(EXTERNAL_NORFLASH_PORT12_15_SSP2, pBuffer, tts_address+iPos, nSize);
	return 1;
}
#endif

//int latency2=7;
int mp3_file_play(char *file_name)
{
	Mp3PlaybackHandle MP3handle;

	Mp3PlayConfigInfo mp3_configInfo = {
		0,//x01ll<<46,
		0,//10*1024,//0,
		0,
		0,
		Mp3PlaybackEvent_cd,
		0
	};
	AUDIO_PLAY_OPTION test;
	test.option=0;
	test.latency2=7;
	test.write_mode=1;
	test.mode = 2;
//unsigned long long test=0x01ll<<46;
	LOG_INFO("%s: mp3_configInfo.option: 0x%016llX\n", __func__,test.option);
	mp3_configInfo.option=test.option;
	LOG_INFO("%s: mp3_configInfo.option: 0x%016llX\n", __func__,mp3_configInfo.option);
	audio_play_state = MP3_PLAYBACK_STATUS_STARTED;
	ql_play_mp3(file_name,&MP3handle,&mp3_configInfo);

	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	LOG_DBG("%s: start\n",__func__);

	for (int i = 0; i < delay_times; i++)
	{
	
		if ((audio_play_state == MP3_PLAYBACK_STATUS_ENDED))
		{
			break;
		}
		ql_rtos_task_sleep_ms(50);
	}
	ql_stop_mp3_play(MP3handle,0);
}

static void WAVPlaybackEvent_cd(acm_audio_track_handle handle,acm_audio_track_event_t event)
{
//	LOG_INFO("%s: MP3_CD--event: %d\n", __func__,event);

	if(event == AUDIO_TRACK_EVENT_STARTED)
	{
//		printf("%s start\r\n",__func__);
	}
	if(event == AUDIO_TRACK_EVENT_CLOSED)
	{
		audio_play_state = event;
//		printf("%s end\r\n",__func__);        
	}

}

	/** pcm channel, non-zero expected*/
	//unsigned int channels;
	/** pcm sample rate, non-zero expected*/
	//unsigned int rate;
	/** audio track capacity, how many microsecond of pcm can be held*/
	//unsigned int capacity;
	/** audio track start threshold, how many microsecond of pcm is held before involving in schedule*/
	//unsigned int start_threshold;
	/** audio track prepare threshold, how many microsecond of pcm is held before playing out*/
	//unsigned int prepare_threshold;
	/** initial gain in q8 format, 0db by default*/
	//int gain_value;
	/** initial speed factor in q8 format, x1 by default*/
	//int speed_factor;
	/** eq effect index, 0 for none, minus 1 for lookup*/
	//int eq;
	/** playback property in bitmap format, refer @ref AUDIO_PLAY_OPTION*/
	//unsigned long long option;
	/** enhance in dsp further more if any*/
	//int dsp_mode;
	/** override voice call data if any*/
	//int override_voice;
	/** pcm format, 16 bit depth little endian by default*/
	//acm_audio_track_format_t format;
	/** schedule mode, combine mode by default*/
	//acm_audio_track_mode_t mode;
	/** audio track direction, near side by default*/
	//acm_audio_track_direction_t direction;
	/** audio track event callback*/
	//acm_audio_track_event_callback_t event_cb;
	/** audio track write mode, 0 for non-block*/
	//acm_audio_track_write_mode_t write_mode;
	/** turn off audio effect for gain etc*/
	//unsigned int effect_off;
static audio_track_handle  wav_handle;
static struct audio_track_config wav_configInfo;

int WAV_file_play(char *file_name)
{
	LOG_INFO("%s: start:%s\n",__func__,file_name);
	audio_play_state = AUDIO_TRACK_EVENT_STARTED;
	int ret = 0;
	AUDIO_PLAY_OPTION prop = {0};
	prop.latency2 = 4;

	wav_configInfo.option = prop.option;
	wav_configInfo.event_cb=WAVPlaybackEvent_cd;
	ret = ql_wav_play(file_name,&wav_configInfo,&wav_handle);
	LOG_INFO("%s: ret %d\n",__func__,ret);
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;
	
	for (int i = 0; i < delay_times; i++)
	{
		if ((audio_play_state == AUDIO_TRACK_EVENT_CLOSED))
		{
			break;
		}
		ql_rtos_task_sleep_ms(50);
	}
	ql_wav_play_stop(wav_handle,0);

}



void openOffSpeaker(char status)
{
#if 0
	if(status == 0)
		gpio_speaker_onoff(0);
	if(status == 1)
		gpio_speaker_onoff(1);
#endif
}

void tts_wait_card_register(void)
{
    int wait_card_count = 0;

    while(1)
    {
        if (wait_card_count > WAIT_NETWORK_REGISTER_MAX_COUNT)
        {
            break;
        }

        if (g_network_register_status == 0)
        {
            wait_card_count++;
            ql_rtos_task_sleep_ms(100);
            continue;
        }
        else
        {
            ql_rtos_task_sleep_ms(500);
            break;
        }
    }

    return ;
}

int tts_get_status(void)
{
    return tts_play_status;
}

void print_hex(const char *title, const unsigned char buf[], size_t len)
{
#if 0
    usb_log_printf("%s: ", title);

    for (size_t i = 0; i < len; i++)
    {
        if (!(i%16))
    		usb_log_printf("\r\n");
        usb_log_printf("%02x", buf[i]);
    }

    usb_log_printf("\r\n");
#else
  usb_log_printf("%s len %d: \r\n", title,len);
	while(len)
	{
		size_t ii;
		size_t cnt;
		cnt=len;
		if (cnt>16) cnt=16;
		for(ii=0;ii<cnt;ii++)
			usb_log_printf("%02x ", buf[ii]);
		for(;ii<16;ii++)
			usb_log_printf("   ");
		usb_log_printf("\t:");
		for(ii=0;ii<cnt;ii++)
		{
			if (buf[ii]<0x20)
				usb_log_printf(".");
			else
				usb_log_printf("%c",buf[ii]);
		}
		buf +=cnt;
		len -=cnt;
		usb_log_printf("\r\n");
	}
#endif
}

void WaitAudioPlayOver(void)
{
//    printf("---------%d\r\n",__LINE__);
    if (xSemaphore != NULL)
    {
    	ql_rtos_mutex_lock(xSemaphore, 0xFFFFFFFF);

    	ql_rtos_mutex_unlock(xSemaphore);
    }
}

void WaitAudioOver(void)
{
    unsigned int queue_cnt = 0;
    uint32_t wait_timeout = ql_rtos_get_systicks_to_ms() + 30 * 1000;
    do
    {
        ql_rtos_queue_get_cnt( ttsPlayQueue, &queue_cnt );
        if( ql_rtos_get_systicks_to_ms() > wait_timeout )
        {
            LOG_INFO("%s: wait timeout\n",__func__);
            break;
        }
        ql_rtos_task_sleep_ms(500);
    } while ( queue_cnt > 0 );

    WaitAudioPlayOver();

    LOG_INFO("%s: done\n",__func__);
}

void AudioPlayHalt(void)
{
	//δ��ʼ������£���AudioPlayContinue�첽���ã�����б�ʱ���ܵ����쳣
	if (xSemaphore==NULL)
		ql_rtos_mutex_create(&xSemaphore);

	ql_rtos_mutex_lock(xSemaphore, 0xFFFFFFFF);
}
void AudioPlayContinue(void)
{
	ql_rtos_mutex_unlock(xSemaphore);
}

void audio_clear_all_msg(void)
{
}


void tts_play_cb(QL_TTS_ENVENT_E event, char *str)
{
	LOG_INFO("%s_%d === :event = %d,str = %s\n", __func__,__LINE__,event,str);

	if (event == QL_TTS_EVT_PLAY_START)
	{
		tts_play_state = event;
	}
	else if (event == QL_TTS_EVT_PLAY_FINISH)
	{
		tts_play_state = event;
	}
}

int tts_cb_mp3_file_play(char *p_data, int len, int res) //p_data and len is not usd
{
//    LOG_INFO("%s: audio_play_state = %d\n", __func__,res);

	audio_play_state = res;

	return 0;
}

void speakerpa_handle_cb(UINT32 on)
{
	if(on == 1)
	{
//		LOG_INFO("%s_%d =========\n", __func__, __LINE__);
		gpio_speaker_onoff(1);
	}
	else
	{
//		LOG_INFO("%s_%d ==========\n", __func__, __LINE__);
		gpio_speaker_onoff(0);
	}
//	ql_rtos_task_sleep_ms(40);
}

enum{
	GBK_TEXT,
	UNICODE_TEXT,
	UTF8_TEXT
};

static void tts_check_play_status(void)
{
	int i = 0;
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;

	for (i = 0; i < delay_times; i++)
	{
		ql_rtos_task_sleep_ms(50);

		if ((tts_play_state == QL_TTS_EVT_PLAY_FINISH) || (tts_play_state == QL_TTS_EVT_PLAY_FAILED))
		{
			ql_rtos_task_sleep_ms(50);
			break;
		}
	}
}

int Module_TTS_Play_Text(char * tts_buf,int type,void (*Switch)(uint8_t OnOff))
{
	QlOSStatus ret = 0;
	//int i = 0;
	int tts_play_len = 0;
	unsigned short* unicode_buf  = NULL;
	uint8_t * tts_play_buf=NULL;

	LOG_INFO("%s_%d play tts type:%d,str:%s \n", __func__, __LINE__,type,tts_buf);

	LOG_INFO("%s : start\n",__func__);
	openOffSpeaker(1);
	if (type == GBK_TEXT)
	{
		if (audio_file_play_flag == 1)
		{
			//ql_stop_mp3_play();
			audio_file_play_flag = 0;
		}

		tts_play_state = QL_TTS_EVT_PLAY_START;
//		LOG_INFO("%s_%d play \"%s\":\n", __func__, __LINE__,tts_buf);
		ql_tts_play(2, tts_buf);

		tts_check_play_status();
	}
	else if (type == UNICODE_TEXT)
	{
		tts_play_state = QL_TTS_EVT_PLAY_START;
		ql_tts_play(1, tts_buf);

		tts_check_play_status();
	}
	else if (type == UTF8_TEXT)
	{
		if (audio_file_play_flag == 1)
		{
			//ql_play_mp3_stream_drain();
			//ql_play_mp3_stream_stop();
			audio_file_play_flag = 0;
		}
		unicode_buf = UTF8_To_Unicode(tts_buf, strlen(tts_buf), &tts_play_len);
		if(unicode_buf==NULL)
		{
			ret=-1;
			goto exit;
		}
		tts_play_buf=malloc(tts_play_len*4+1);
		if(tts_play_buf==NULL)
		{
			free(unicode_buf);
			ret=-1;
			goto exit;
		}
		memset(tts_play_buf, 0, tts_play_len*4+1);
		Hex_To_String(tts_play_buf, unicode_buf, tts_play_len);
		tts_play_state = QL_TTS_EVT_PLAY_START;
		ql_tts_play(1, tts_play_buf);

		tts_check_play_status();

		if (unicode_buf != NULL)
		{
			free(unicode_buf);
		}

		if (tts_play_buf != NULL)
		{
			free(tts_play_buf);
		}
	}

exit:
	openOffSpeaker(0);

	return ret;
}

int Module_TTS_Play_Factory_Text(char * tts_buf,void (*Switch)(uint8_t OnOff))
{
	return Module_TTS_Play_Text(tts_buf,GBK_TEXT,Switch);
}

static void ql_audio_track_callback(acm_audio_track_handle handle, acm_audio_track_event_t event)
{
	printf("audio_track_callback event:%d\r\n", event);
}
//void pcmPlayMemory(struct tts_buff_msg * msgin)
void pcmPlayMemory(const char * pData,unsigned long mLen)
{
	audio_track_handle track_hd;
	unsigned int rate = 16000; 
//	unsigned int channels = 2;
	unsigned int channels = 1;
	int write_ret=-1;

	//int delay_times = AUDIO_WAIT_DELAY_TIMES;
	unsigned char *audio_data_buf = NULL;
	int ii = 0;
	AUDIO_PLAY_OPTION option;
	option.option=0x100;
	option.latency2 = 4;
	option.write_mode=AUDIO_TRACK_WRITE_MODE_SYNC;

	mLen &=~1;
		
	openOffSpeaker(1);

//	ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
#if 1
	track_hd = ql_audio_track_open(rate, channels, option.option, NULL);
	if (track_hd == NULL) {
		LOG_INFO("ql_pcm_open write fail\n");
		return;
	}
	write_ret=ql_audio_track_write(track_hd, pData, mLen);		  
	LOG_INFO("%s_%d ===pcm write:%d\n", __func__, __LINE__,write_ret);
#else
	for (ii=0; ii<400; ii++)
	{
		audio_data_buf = malloc(AUDIO_MP3_MEMORY_BUF_LEN);
		if(audio_data_buf != NULL) 
		{
			break;
		}
		ql_rtos_task_sleep_ms(5);
	}

	if(audio_data_buf == NULL) 
	{
		LOG_INFO("%s_%d ===audio_data_buf malloc error\n", __func__, __LINE__);
		return;
	}

//	track_hd = ql_audio_track_open(rate, channels, 0x100, ql_audio_track_callback);
	track_hd = ql_audio_track_open(rate, channels, option.option, NULL);
	if (track_hd == NULL) {
		LOG_INFO("ql_pcm_open write fail\n");
		if (audio_data_buf != NULL)
		{
			free(audio_data_buf);
			audio_data_buf = NULL;
		}
		return;
	}

	audio_file_play_flag = 1;
//	int flag=0;
	while(mLen)
	{
		if(mLen > AUDIO_MP3_MEMORY_PLAY_LEN)
		{
			memset(audio_data_buf, 0, AUDIO_MP3_MEMORY_BUF_LEN);
			memcpy(audio_data_buf, pData, AUDIO_MP3_MEMORY_PLAY_LEN);

			//ql_pcm_write(write_hdl, audio_data_buf, AUDIO_MP3_MEMORY_PLAY_LEN);
			//write_ret = ql_audio_track_write(track_hd, tmp_buf, record_len);		  
			write_ret=ql_audio_track_write(track_hd, audio_data_buf, AUDIO_MP3_MEMORY_PLAY_LEN);		  
			LOG_INFO("%s_%d ===pcm write:%d\n", __func__, __LINE__,write_ret);

			mLen -= AUDIO_MP3_MEMORY_PLAY_LEN;
			pData += AUDIO_MP3_MEMORY_PLAY_LEN;
		}
		else
		{
			memset(audio_data_buf, 0, AUDIO_MP3_MEMORY_BUF_LEN);
			memcpy(audio_data_buf, pData, mLen);
			//ql_pcm_write(write_hdl, audio_data_buf, mLen);
			write_ret=ql_audio_track_write(track_hd, audio_data_buf, mLen);		  
			LOG_INFO("%s_%d ===pcm write:%d\n", __func__, __LINE__,write_ret);

			pData +=mLen;
			mLen=0;
		}
	}

	if (audio_data_buf != NULL)
	{
		free(audio_data_buf);
		audio_data_buf = NULL;
	}
#endif    
		
	ql_audio_track_close(track_hd, 0);
	audio_file_play_flag = 0;
	openOffSpeaker(0);
}

void mp3PlayMemory(const char * pData,unsigned long mLen)
{
	int i = 0;
	mp3_stream_play(pData, mLen);
	LOG_DBG("%s: finish\n",__func__);
}

int tts_play_local_file(const char* fpath)
{
	int ret;
	char filetype = 0;
	char path[64] = {0};
	int delay_times = AUDIO_WAIT_DELAY_TIMES/50;

//	LOG_INFO("%s_%d ===============\n", __func__, __LINE__);
	if( 0 == memcmp(fpath,AUDIO_RESOURCE_ROOT_PATH,strlen(AUDIO_RESOURCE_ROOT_PATH)) )
	{
		// full path
		snprintf(path,sizeof(path),"%s",fpath);
	}
	else
	{
		snprintf(path,sizeof(path),"%s%s",VOICE_PATH_PREFIX,fpath);
		ret = ql_access(path,0);
		if ( ret != 0 )
		{
			// fallback to U:/
			memset(path,0,sizeof(path));
			snprintf(path,sizeof(path),"%s%s",AUDIO_RESOURCE_ROOT_PATH,fpath);
		}
	}

	ret = ql_access(path,0);
	if( ret != 0 )
	{
		usb_log_printf("%s: file %s is not access\n",__func__,path);
		return ret;
	}

	LOG_INFO("%s: play %s\n",__func__,path);

	int path_len = strlen(path);
	if( path_len > 4 )
	{
		if( 0 == memcmp(path+path_len-4,".amr",strlen(".amr")) )
		{
			filetype = 1;
		}
		else if( 0 == memcmp(path+path_len-4,".wav",strlen(".wav")) )
		{
			filetype = 2;
		}
	}

//	LOG_INFO("%s_%d ===============\n", __func__, __LINE__);

	if( filetype == 1 )
	{
		LOG_INFO("%s: audio file type AMR\n",__func__);
		amr_file_play(path);
	}
	else if( filetype == 2 )
	{
		LOG_INFO("%s: audio file type WAV\n",__func__);
		WAV_file_play(path);
		LOG_INFO("%s_%d ===============\n", __func__, __LINE__);
	}
	else
	{
		LOG_INFO("%s: audio file type MP3\n",__func__);
		mp3_file_play(path);
	}

	LOG_INFO("%s: end\n",__func__);
	return 0;
}

int TTsPlayByResInfo(const AudioResInfo *info)
{
	if (info==NULL)
		return -1;

	LOG_INFO("%s_%d play \"%s\":\n", __func__, __LINE__,info->TTSstr);
	if (info->filePinyinName)
	{
		//tts_play_local_file(info->filePinyinName);
		if (!tts_play_local_file(info->filePinyinName))
			return 0;
		else
		{
			LOG_INFO("%s_%d play file(%s) fail!\n", __func__, __LINE__,info->filePinyinName);
		}
	}	
	
	if (info->Array)
	{
		if (!memcmp(info->Array->Data,"ID",2))
			mp3PlayMemory(info->Array->Data,info->Array->dlen);
		else
			pcmPlayMemory(info->Array->Data,info->Array->dlen);
	}
	else
	{
		Module_TTS_Play_Text(info->TTSstr, GBK_TEXT,gpio_speaker_onoff);
		LOG_INFO("%s_%d play tts over!\n", __func__, __LINE__);
//		return -1;
	}

	return 0;
}

int TTsPlayGroup(int size ,short const * group)
{
	int ii;
	AudioResInfo * info;

	if (group==NULL)
		return -1;

#if 0		
	for(ii=0;ii<size;ii++)
	{
		if (TTsPlayByResInfo(GetRecInfo(group[ii])))
		{
			LOG_INFO("%s_%d group idx=%d play err!\n", __func__, __LINE__,group[ii]);
//			return -1;
		}
	}
#else
	ii=0;
	while(size)
	{
		if (ii<size)
		{
			info=GetRecInfo(group[ii]);
			if (info!=NULL)
			{
				if (info->filePinyinName!=NULL)
				{
					char sbuf[5];
					char const * ptr=info->filePinyinName+strlen(info->filePinyinName);
					ptr -=3;
					memset(sbuf,0,sizeof(sbuf));
					memcpy(sbuf,ptr,3);
					strupr(sbuf);
					if (!memcmp(sbuf,"WAV",3))
					{
						ii++;
						continue;
					}
				}
			}
			else
			{
				LOG_INFO("%s_%d group idx=%d play err!\n", __func__, __LINE__,group[ii]);
	//			return -1;
			}
		}
		
//			LOG_INFO("%s_%d playlist cnt %d start idx=%d\n", __func__, __LINE__,ii,*group);
		if ((ii==0)||(ii==1))
		{
			if (TTsPlayByResInfo(GetRecInfo(*group)))
			{
				LOG_INFO("%s_%d group idx=%d play err!\n", __func__, __LINE__,group[ii]);
	//			return -1;
			}
			group++;
			size--;
		}
		else
		{//playlist 
			LOG_INFO("%s_%d playlist cnt %d start idx=%d\n", __func__, __LINE__,ii,*group);
			wav_mix_play_list(ii,group);
			group+=ii;
			size -=ii;
		}
		ii=0;
	}
#endif

	return 0;
}

int TTsPlaySaleId(SaleInfo * pSaleInfo)
{
	short const payIdx[]=
	{
		AUD_ID_PAYMENT,
		AUD_ID_PAYMENT,
		AUD_ID_PAYMENT,
		AUD_ID_PAYMENT,
		AUD_ID_PAYMENT,
	};
	short group[30];
	short * pIdx;
	int idx;

	if (pSaleInfo==NULL)
		return -1;

	if( pSaleInfo->Amount )
	{
		disp_set_paymsg( pSaleInfo->Amount, pSaleInfo->timestamp, TTS_FLAG_PAYMSG | TTS_FLAG_FLASH );
		disp_onoff_request( 1, DISP_HOLDON_MS );
		disp_show_flash( FG00_FLASH_ALL, 3, 500 );
		ql_rtos_task_sleep_ms(100);
	}
	else //if( reviceMsg.flag & TTS_FLAG_CANCEL )
	{
		disp_set_paymsg(0,pSaleInfo->timestamp,TTS_FLAG_CANCEL);
		disp_onoff_request(1,DISP_HOLDON_MS);
		disp_show_flash( FG00_FLASH_ALL, 3, 500 );
	}

	if (pSaleInfo->SalePlayId>=(sizeof(payIdx)/sizeof(payIdx[0])))
	{
		LOG_INFO("%s: SalePlayId=%d not found\n",__func__,pSaleInfo->SalePlayId);
		return -1;
	}
		
	pIdx=group;
	*pIdx++=payIdx[pSaleInfo->SalePlayId];

	idx=num_to_audio_idx(pSaleInfo->Amount,pIdx,sizeof(group)/sizeof(group[0]) -1);
	if (idx<0)
	{
		LOG_INFO("%s_%d === num to idx ret %d\n", __func__,__LINE__,idx);
		return -1;
	}
		
	pIdx +=idx;
	*pIdx++=AUD_ID_UNIT;
	
//	return TTsPlayGroup((pIdx-group)/sizeof(group[0]),group);
	return TTsPlayGroup((pIdx-group),group);
}

int TTsPlaySaleStr(SaleInfo * pSaleInfo)
{

	const char *pay[]=
	{
		PAY_MSG_SK,
	};
	short const payIdx[]=
	{
		AUD_ID_PAYMENT,
	};
	char const Dollar[] = PAY_MSG_DOLLAR;	

	int index;
	short group[30];
	short * pIdx;
	char * ptr;

	if (pSaleInfo==NULL)
		return -1;

	if( pSaleInfo->Amount )
	{
		disp_set_paymsg( pSaleInfo->Amount, pSaleInfo->timestamp, TTS_FLAG_PAYMSG | TTS_FLAG_FLASH );
		disp_onoff_request( 1, DISP_HOLDON_MS );
		disp_show_flash( FG00_FLASH_ALL, 3, 500 );
	}
	else //if( reviceMsg.flag & TTS_FLAG_CANCEL )
	{
		disp_set_paymsg(0,pSaleInfo->timestamp,TTS_FLAG_CANCEL);
		disp_onoff_request(1,DISP_HOLDON_MS);
		disp_show_flash( FG00_FLASH_ALL, 3, 500 );
	}
	
	index=0;
	for(index=0;index<(sizeof(pay)/sizeof(pay[0]));index++)
	{
		int len=strlen(pay[index]);
		if (!memcmp(pSaleInfo->msg, pay[index],len))
		{
			LOG_INFO("%s: pay mode match %d\n",__func__,index);
			break;
		}	
	}

	if ( index>=(sizeof(pay)/sizeof(pay[0]) ))
	{
		LOG_INFO("%s: no pay mode match\n",__func__);
		return -1;
	}
	
	ptr = strstr(pSaleInfo->msg+strlen(pay[index]), Dollar);
	if (ptr == NULL) 
	{
		LOG_INFO("%s_%d === not pay str\n", __func__,__LINE__);
		return -1;
	}
	* ptr=0;

	pIdx=group;
	*pIdx++= AUD_ID_PAYMENT;

	index=numstr_to_audio_idx(pSaleInfo->msg+strlen(pay[index]),pIdx,sizeof(group)/sizeof(group[0]) -1);
	if (index<0)
	{
		LOG_INFO("%s_%d === str to idx ret %d\n", __func__,__LINE__,index);
		return -1;
	}
	pIdx +=index;	
	*pIdx++=AUD_ID_UNIT;
//		LOG_INFO("%s_%d === ret %d\n", __func__,__LINE__,(pIdx-group));
	return TTsPlayGroup((pIdx-group),group);
}

int TTsPlaySale(int type,SaleInfo * priv)
{
	char const * msgid;
	char const * url;
	int ret=-1;
	
	if (type==AUD_ID_RECODE_PLAY)
	{
		int recIdx=(int)priv->msgid;
		priv->msgid=NULL;
		if (TTsPlayRecIdx(recIdx))
		{
			return -1;
		}

		return TTsPlaySaleStr(priv);
	}

	msgid=priv->msgid;
	url=priv->url;

	if (type==AUD_ID_SALE_STR_INFO)
		ret=TTsPlaySaleStr(priv);
	else
		return -1;
	
	if (url!=NULL)
	{
		play_receipt_set(url, strlen(url), msgid, strlen(msgid));
	}
	return ret;
}

int TTsPlayRecIdx(	int recIdx)
{
	short group[30];
	short * pIdx;
//	SaleStrInfo sale;

	pIdx=group;
	if (recIdx&0x80)
	{
		*pIdx++=AUD_ID_RECODE_LAST;
		recIdx &=~0x80;
	}
	else if (recIdx<=1)
	{
		*pIdx++=AUD_ID_RECODE_FIRST;
	}
	else
	{
	//��XX��
	*pIdx++=AUD_ID_RECORD_DI;//��
	recIdx=num_to_audio_idx(recIdx*100,pIdx,sizeof(group)/sizeof(group[0]) -1);
	if (recIdx<0)
	{
		LOG_INFO("%s_%d === num to idx ret %d\n", __func__,__LINE__,recIdx);
		return -1;
	}
	pIdx +=recIdx;
	*pIdx++=AUD_ID_RECORD_BI;//��

	}
	
	recIdx=TTsPlayGroup((pIdx-group),group);
	if (recIdx<0)
		return -1;
		
	return 0;
}


typedef struct
{
	uint8_t VolumeStepTab[SPEADKERS_VOLUME_SET_MAX+1];
}StructAudioSetParam;
StructAudioSetParam AudioSetCS10=
{
	.VolumeStepTab={
		AUDIOHAL_SPK_MUTE,
		AUDIOHAL_SPK_VOL_2, //82db
		AUDIOHAL_SPK_VOL_5, //94db
		AUDIOHAL_SPK_VOL_8, //105db
		AUDIOHAL_SPK_VOL_10, //110db
	},//
};

const uint8_t AudioSetdef[]={		
		AUDIOHAL_SPK_MUTE,
		AUDIOHAL_SPK_VOL_2, 
		AUDIOHAL_SPK_VOL_5, 
		AUDIOHAL_SPK_VOL_8, 
		AUDIOHAL_SPK_VOL_10, 
		};
const uint8_t AudioSetCS62[]={		
		AUDIOHAL_SPK_MUTE,
		AUDIOHAL_SPK_VOL_1,
		AUDIOHAL_SPK_VOL_2,
		AUDIOHAL_SPK_VOL_4, 
		AUDIOHAL_SPK_VOL_5,
		AUDIOHAL_SPK_VOL_7, 
		AUDIOHAL_SPK_VOL_9,
		};


StructAudioSetParam * AudioSetParam=&AudioSetCS10;
StructAudioParam AudioParam;

void AuidoVolumeSet(int CurSet)
{
	if (CurSet>AudioParam.audio_max_level)
		CurSet=AudioParam.audio_max_level;
	
	LOG_INFO("%s_%d === ql  audio %d level=%d\n", __func__,__LINE__,CurSet,*((uint8_t *)AudioParam.AudioTap+CurSet));
	ql_set_volume(*((uint8_t *)AudioParam.AudioTap+CurSet));
	sysparam_get()->volume=CurSet;
	sysparam_save();
}

void audio_tts_init(void)
{
	LOG_INFO("%s_%d === start tts init!\n", __func__,__LINE__);

	AudioParam.AudioTap=AudioSetdef;
	AudioParam.audio_max_level=sizeof(AudioSetdef)/sizeof(AudioSetdef[0])-1;
	LOG_INFO("%s_%d === \n", __func__,__LINE__);
	while ( !ql_get_audio_state( ) )
	{
		ql_rtos_task_sleep_ms( 50 );
	}

	ql_set_audio_path_receiver();

	ql_tts_init(tts_play_cb);

#ifdef TTS_SEPARATE_USE
	unsigned int tts_size = 0;
	ql_norflash_get_addr("quectel_resource_tts", &tts_address, &tts_size);
	tts_address &= 0x0FFFFFFF;
	if(tts_address != NULL){
		LOG_INFO("quectel_resource_tts adress:0x%x, size:%d \r\n",tts_address, tts_size);
		ql_tts_separate_callback(read_cb);
	}
	else
		LOG_INFO("============error!! no tts resource============\r\n");
#endif

	ql_bind_speakerpa_cb(speakerpa_handle_cb);
	if(get_device_config()->id==ID_DS10AK)
	{
		ql_codecpcm_config pcm_config[]={PCM_FS_2048,PCM_CODEC_SAMPLE_48000,1};
		ql_codec_choose(AUD_EXTERNAL_CODEC, &pcm_config);
	}
	else
		ql_codec_choose(AUD_INTERNAL_CODEC, NULL);

	ql_set_txcodecgain(4);
	ql_set_txdspgain(12);

	if (sysparam_get()->volume > AudioParam.audio_max_level)
	{
		sysparam_get()->volume = AudioParam.audio_max_level;
		sysparam_save();
	}

	if (sysparam_get()->volume < 1 )
	{
		sysparam_get()->volume = 1;
		sysparam_save();
	}

	AuidoVolumeSet(sysparam_get()->volume);

	LOG_DBG("%s_%d === finished tts init!\n", __func__,__LINE__);
}

void xr_tts_decode_task(void *pvParameters)
{
	struct AudioPlayMsg reviceMsg;
	uint32_t ret;
	int flag;
	uint32_t play_flag_value;

	flag=0;
	openOffSpeaker(0);
	IdxRangChk();
	ql_tts_set_volume(32765); /* -32768~32767 */

	while(1)
	{
		tts_play_status = 1; // ready
		if ( flag )
		{
			flag = 0;
			ql_rtos_mutex_unlock( xSemaphore );
			ql_rtos_task_sleep_ms(50);
		}
		if ( ql_rtos_queue_wait( vKeyQueue, (u8*) &reviceMsg, sizeof(struct AudioPlayMsg), QL_NO_WAIT ) != OS_OK )
		{
			if ( ql_rtos_queue_wait( ttsPlayQueue, (u8*) &reviceMsg, sizeof(struct AudioPlayMsg), QL_NO_WAIT ) != OS_OK )
			{
				lpm_set( LPM_LOCK_TTS, 0 );
				ret = ql_rtos_flag_wait( play_request_flag, PLAY_TYPE_MASK, QL_FLAG_OR_CLEAR, &play_flag_value, 3000 );
				if ( 0 == ret )
				{
					LOG_INFO( "%s: play request flag 0x%02X\n", __func__, play_flag_value );
				}
				continue;
			}
		}
		if (!flag)
		{
			flag = 1;
			ql_rtos_mutex_lock(xSemaphore, 0xFFFFFFFF);
		}
		tts_play_status = 2; // busy

		lpm_set(LPM_LOCK_TTS,1);

		if ((reviceMsg.Idx>AUD_ID_FIX_AUDIO_BASE)&&(reviceMsg.Idx<AUD_ID_MAX))
		{
			if (TTsPlayByResInfo(GetRecInfo(reviceMsg.Idx)))
				LOG_INFO("%s_%d play idx=%d play fail!\n", __func__, __LINE__,reviceMsg.Idx);
		}
		else switch(reviceMsg.Idx)
		{
			case AUD_ID_GROUP_IDX:
				{
					AudioGroup *group=reviceMsg.priv;
					if (TTsPlayGroup(group->size,group->Idx))
						LOG_INFO("%s_%d play idx=%d err!\n", __func__, __LINE__,reviceMsg.Idx);
				}
				break;
			case AUD_ID_RECODE_PLAY:
			case AUD_ID_SALE_ID_INFO:
			case AUD_ID_SALE_STR_INFO:
				if (!TTsPlaySale(reviceMsg.Idx,reviceMsg.priv))
					break;
			default:
				LOG_INFO("%s_%d play idx=%d err!\n", __func__, __LINE__,reviceMsg.Idx);
				break;
		}

		if (reviceMsg.priv)
			free(reviceMsg.priv);
	}
}

int tts_play_set_idx(int idx, int BreakEna,int playnow )
{
	struct AudioPlayMsg msg;
	struct AudioPlayMsg recv_msg;

	if(audio_pause_play)
		return 0;
		
	if (ttsPlayQueue == NULL)
	{
		return -1;
	}

	LOG_INFO("%s_%d set idx %d\n", __func__, __LINE__,idx);

	memset(&msg,0,sizeof(struct AudioPlayMsg));
	msg.Idx=idx;
	msg.BreakEna=BreakEna;

	if (playnow)
	{
		unsigned int queue_cnt = 0;
		ql_rtos_queue_get_cnt(vKeyQueue, &queue_cnt);
		for (int i = 0; i < queue_cnt; i++)
		{
			ql_rtos_queue_wait(vKeyQueue, (u8*)&recv_msg, sizeof(struct AudioPlayMsg), QL_NO_WAIT);
			if ( recv_msg.priv != NULL )
					free( recv_msg.priv );
		}

		if (ql_rtos_queue_release(vKeyQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
		{
			LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
			return -1;
		}
	}
	else
		if (ql_rtos_queue_release(ttsPlayQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
	{
		LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
		return -1;
	}
	ql_rtos_flag_release(play_request_flag, PLAY_TYPE_TTS, QL_FLAG_OR);
	return 0;
}

int tts_play_set_group(short const * Group,int size,int BreakEna,int playnow )
{
	struct AudioPlayMsg msg;
	struct AudioPlayMsg recv_msg;
	AudioGroup * info;

	if (ttsPlayQueue == NULL)
	{
		return -1;
	}

	LOG_INFO("%s_%d set Group idx %d\n", __func__, __LINE__,Group[0]);

	info=malloc(sizeof(AudioGroup)+size*sizeof(Group[0]));
	if (info==NULL)
		return -1;

	info->size=size;
	memcpy(info->Idx,Group,size*sizeof(Group[0]));

	memset(&msg,0,sizeof(struct AudioPlayMsg));
	msg.Idx=AUD_ID_GROUP_IDX;
	msg.BreakEna=BreakEna;
	msg.priv=info;

	if (playnow)
	{

		unsigned int queue_cnt = 0;
		ql_rtos_queue_get_cnt(vKeyQueue, &queue_cnt);
		for (int i = 0; i < queue_cnt; i++)
		{
			ql_rtos_queue_wait(vKeyQueue, (u8*)&recv_msg, sizeof(struct AudioPlayMsg), QL_NO_WAIT);
			if ( recv_msg.priv != NULL )
					free( recv_msg.priv );
		}

		if (ql_rtos_queue_release(vKeyQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
		{
			free(info);
			LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
			return -1;
		}
	}
	else
		if (ql_rtos_queue_release(ttsPlayQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
	{
		free(info);
		LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
		return -1;
	}
	ql_rtos_flag_release(play_request_flag, PLAY_TYPE_TTS, QL_FLAG_OR);
	return 0;
}

int tts_play_set_sale_id(const char * msgid,const char * url,int SalePlayId,uint64_t Amount,uint32_t timestamp)
{
	struct AudioPlayMsg msg;
	SaleInfo * info;
	int len;

	if (ttsPlayQueue == NULL)
	{
		return -1;
	}

	LOG_INFO("%s_%d set sale SalePlayId %d\n", __func__, __LINE__,SalePlayId);
	len=sizeof(SaleInfo);
	if (msgid!=NULL) len +=strlen(msgid)+1;
	if (url!=NULL) len +=strlen(url)+1;
	info=malloc(len);
	if (info==NULL)
		return -1;

	memset(info,0,len);
	info->SalePlayId=SalePlayId;
	info->Amount=Amount;
	info->timestamp=timestamp;
	len=0;
	if ((msgid==NULL)||(!strlen(msgid))) info->msgid=NULL;
	else
	{
		info->msgid=info->sbuf;
		len +=strlen(msgid)+1;
		strcpy(info->msgid,msgid);
	}
	if ((url==NULL)||(!strlen(url))) info->url=NULL;
	else
	{
		info->url=&info->sbuf[len];
		len +=strlen(url)+1;
		strcpy(info->url,url);
	}

	memset(&msg,0,sizeof(struct AudioPlayMsg));
	msg.Idx=AUD_ID_SALE_ID_INFO;
	msg.BreakEna=0;
	msg.priv=info;

	if (ql_rtos_queue_release(ttsPlayQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
	{
		free(info);
		LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
		return -1;
	}
	ql_rtos_flag_release(play_request_flag, PLAY_TYPE_TTS, QL_FLAG_OR);
	//printf("%s OS_QueueSend send ok\n", __func__);
	return 0;
}

int tts_play_set_sale_str(const char * msgid,const char * url,char const * salemsg,uint64_t Amount,uint32_t timestamp)
{
	struct AudioPlayMsg msg;
	SaleInfo * info;
	int len;

	if (ttsPlayQueue == NULL)
	{
		return -1;
	}

	LOG_INFO("%s_%d set sale msg %s\n", __func__, __LINE__,salemsg);
	len=sizeof(SaleInfo);
	if (msgid!=NULL) len +=strlen(msgid)+1;
	if (url!=NULL) len +=strlen(url)+1;
	if (salemsg!=NULL) len +=strlen(salemsg)+1;
	info=malloc(len);
	if (info==NULL)
		return -1;

	memset(info,0,len);
	info->Amount=Amount;
	info->timestamp=timestamp;
	len=0;
	if ((msgid==NULL)||(!strlen(msgid))) info->msgid=NULL;
	else
	{
		info->msgid=info->sbuf;
		len +=strlen(msgid)+1;
		strcpy(info->msgid,msgid);
	}
	if ((url==NULL)||(!strlen(url))) info->url=NULL;
	else
	{
		info->url=&info->sbuf[len];
		len +=strlen(url)+1;
		strcpy(info->url,url);
	}
	if ((salemsg==NULL)||(!strlen(salemsg))) info->msg=NULL;
	else
	{
		info->msg=&info->sbuf[len];
		len +=strlen(salemsg)+1;
		strcpy(info->msg,salemsg);
	}

	memset(&msg,0,sizeof(struct AudioPlayMsg));
	msg.Idx=AUD_ID_SALE_STR_INFO;
	msg.BreakEna=0;
	msg.priv=info;

	if (ql_rtos_queue_release(ttsPlayQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
	{
		free(info);
		LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
		return -1;
	}
	ql_rtos_flag_release(play_request_flag, PLAY_TYPE_TTS, QL_FLAG_OR);
	//printf("%s OS_QueueSend send ok\n", __func__);
	return 0;
}

int tts_play_set_record_idx(int recidx,char const * salemsg,uint64_t Amount,uint32_t timestamp)
{
	struct AudioPlayMsg msg;
	struct AudioPlayMsg recv_msg;
	SaleInfo * info;
	int len;

	if (ttsPlayQueue == NULL)
	{
		return -1;
	}

	LOG_INFO("%s_%d set sale msg %s\n", __func__, __LINE__,salemsg);
	len=sizeof(SaleInfo);
	if (salemsg!=NULL) len +=strlen(salemsg)+1;
	info=malloc(len);
	if (info==NULL)
		return -1;

	memset(info,0,len);
	info->Amount=Amount;
	info->timestamp=timestamp;
	info->msgid=(void *)recidx;
	len=0;
	if ((salemsg==NULL)||(!strlen(salemsg))) info->msg=NULL;
	else
	{
		info->msg=&info->sbuf[len];
		len +=strlen(salemsg)+1;
		strcpy(info->msg,salemsg);
	}

	memset(&msg,0,sizeof(struct AudioPlayMsg));
	msg.Idx=AUD_ID_RECODE_PLAY;
	msg.BreakEna=0;
	msg.priv=info;

	unsigned int queue_cnt = 0;
	ql_rtos_queue_get_cnt(vKeyQueue, &queue_cnt);
	for (int i = 0; i < queue_cnt; i++)
	{
		ql_rtos_queue_wait(vKeyQueue, (u8*)&recv_msg, sizeof(struct AudioPlayMsg), QL_NO_WAIT);
		if ( recv_msg.priv != NULL )
				free( recv_msg.priv );
	}

	if (ql_rtos_queue_release(vKeyQueue, sizeof(struct AudioPlayMsg), (u8*)&msg, QL_NO_WAIT) != OS_OK)
	{
		free(info);
		LOG_INFO("%s_%d OS_QueueSend send error\n", __func__, __LINE__);
		return -1;
	}
	ql_rtos_flag_release(play_request_flag, PLAY_TYPE_TTS, QL_FLAG_OR);
	//printf("%s OS_QueueSend send ok\n", __func__);
	return 0;
}

void tts_play_init(void)
{
	tts_play_status = 0;
	short group[5];
	short * pGroup;

	audio_tts_init();

	ql_rtos_flag_create(&play_request_flag);
	if (xSemaphore==NULL)
		ql_rtos_mutex_create(&xSemaphore);
	LOG_DBG("%s_%d === xSemaphore create success!\n", __func__,__LINE__);

	if (ql_rtos_queue_create(&vKeyQueue, sizeof(struct AudioPlayMsg), 1) != OS_OK) {

		LOG_INFO("ql_rtos_queue_create create error\n");
		return ;
	}
	LOG_DBG("%s_%d === vKeyQueue create success!\n", __func__,__LINE__);

	if (ql_rtos_queue_create(&ttsPlayQueue, sizeof(struct AudioPlayMsg), 50) != OS_OK) {
		LOG_INFO("OS_QueueCreate create error\n");
		return ;
	}
	LOG_DBG("%s_%d === ttsPlayQueue create success!\n", __func__,__LINE__);

	if (get_factory_test_status() == 0)
	{
		pGroup=group;
		*pGroup++=AUD_ID_WELCOME_1;
		tts_play_set_group(group,pGroup-group,0,0);
	}

	if (ql_rtos_task_create(&tts_decode_task_ctrl_thread,
							15*1024,
							80,
							"xr_tts_decode_task",
							xr_tts_decode_task,
							NULL) != OS_OK) {

		LOG_INFO("thread create error\n");
		return ;
	}

	LOG_INFO("%s_%d === tts_decode_task_ctrl_thread create success!\n", __func__,__LINE__);
}


