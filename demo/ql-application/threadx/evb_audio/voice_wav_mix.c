/**
  ******************************************************************************
  * @file    aliyun_iot_mqtt.c
  * @author  Xu
  * @version V1.0.0
  * @date    2019/03/01
  * @brief   aliyun iot mqtt func file for C code
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_rtos.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_fs.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "tts_yt_task.h"
#include "res.h"
#include "voice_wav_mix.h"

#define LOG_DBG(...)            //do{printf("[DBG WMIX]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{printf("[INFO WMIX]: "); printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    int16_t voice_data;
    int start_offset;
    QFILE *fp;
    uint32_t rcount;
    Wav_t info;
}MixCtrl_t;

/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define WAV_SAMPLE_RATE         16000
#define WAV_MIX_TIME            90
#define MIX_BUFF_SIZE           (20 * 1024)

#define WAV_PLAY_STOP           0
#define WAV_PLAY_WAITING        1
#define WAV_PLAY_PLAYING        2
#define WAV_PLAY_FINISH         3

#define WAV_FILE_PATH_PREFIX    "U:/"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define LITTLE_TO_U32(P)    ((P[3] << 24) + (P[2] << 16) + (P[1] << 8) + P[0])
#define LITTLE_TO_U16(P)    ((P[1] << 8) + P[0])
#define U32_TO_LITTLE(P,B) {B[0] = P&0xff; B[1] = (P >> 8) & 0xff; B[2] = (P >> 16) & 0xff; B[3] = (P >> 24) & 0xff;}
#define U16_TO_LITTLE(P,B) {B[0] = P&0xff; B[1] = (P >> 8) & 0xff; }
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static ql_task_t wav_play_ctrl_task = NULL;
static uint8_t *wav_data_buff = NULL;
static uint8_t *wav_play_buff1 = NULL;
static uint32_t len_play_buff1 = 0;
static uint8_t *wav_play_buff2 = NULL;
static uint32_t len_play_buff2 = 0;
static uint32_t wav_data_len = 0;
static int wav_play_status = 0;
static int wav_mix_duration = WAV_MIX_TIME;
//static PCM_HANDLE_T pcm_write_hdl = NULL;
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#if 1
void show_wav_info(Wav_t *pinfo)
{
    /**
     *  RIFF
     */
    LOG_INFO("ChunkID \t%c%c%c%c\n", pinfo->riff.ChunkID[0], pinfo->riff.ChunkID[1], pinfo->riff.ChunkID[2], pinfo->riff.ChunkID[3]);
    LOG_INFO("ChunkSize \t%ld\n", pinfo->riff.ChunkSize);
    LOG_INFO("Format \t\t%c%c%c%c\n", pinfo->riff.Format[0], pinfo->riff.Format[1], pinfo->riff.Format[2], pinfo->riff.Format[3]);

    LOG_INFO("\n");

    /**
     *  fmt
     */
    LOG_INFO("Subchunk1ID \t%c%c%c%c\n", pinfo->fmt.Subchunk1ID[0], pinfo->fmt.Subchunk1ID[1], pinfo->fmt.Subchunk1ID[2], pinfo->fmt.Subchunk1ID[3]);
    LOG_INFO("Subchunk1Size \t%ld\n", pinfo->fmt.Subchunk1Size);
    LOG_INFO("AudioFormat \t%d\n", pinfo->fmt.AudioFormat);
    LOG_INFO("NumChannels \t%d\n", pinfo->fmt.NumChannels);
    LOG_INFO("SampleRate \t%ld\n", pinfo->fmt.SampleRate);
    LOG_INFO("ByteRate \t%ld\n", pinfo->fmt.ByteRate);
    LOG_INFO("BlockAlign \t%d\n", pinfo->fmt.BlockAlign);
    LOG_INFO("BitsPerSample \t%d\n", pinfo->fmt.BitsPerSample);

    LOG_INFO("\n");

    /**
     *  data
     */
    LOG_INFO("blockID \t%c%c%c%c\n", pinfo->data.Subchunk2ID[0], pinfo->data.Subchunk2ID[1], pinfo->data.Subchunk2ID[2], pinfo->data.Subchunk2ID[3]);
    LOG_INFO("blockSize \t%ld\n", pinfo->data.Subchunk2Size);

    LOG_INFO("\n");

//    duration = Subchunk2Size / ByteRate
    LOG_INFO("duration \t%ldms\n", pinfo->data.Subchunk2Size * 1000 / pinfo->fmt.ByteRate);

    LOG_INFO("\n");
}

int get_wav_info(const char *filename,Wav_t *pinfo)
{
    int ret = 0;
    uint8_t rbuff[64];

    if( !filename || !pinfo )
    {
        LOG_INFO("%s: invalid file name\n",__func__);
        return -1;
    }

    QFILE *fp = ql_fopen(filename,"rb");
    if( !fp )
    {
        LOG_INFO("%s: open %s fail\n",__func__,filename);
        return -1;
    }

    memset(pinfo,0,sizeof(Wav_t));
    // read RIFF
    ret = ql_fread(pinfo->riff.ChunkID, sizeof(pinfo->riff.ChunkID), 1, fp);
    if (ret <= 0)
    {
        LOG_INFO("%s: read RIFF id fail\n", __func__);
        goto get_wav_info_fail;
    }
    ret = ql_fread(rbuff, sizeof(uint32_t), 1, fp);
    if (ret <= 0)
    {
        LOG_INFO("%s: read RIFF size fail\n", __func__);
        goto get_wav_info_fail;
    }
    pinfo->riff.ChunkSize = LITTLE_TO_U32(rbuff);
    ret = ql_fread(pinfo->riff.Format, sizeof(pinfo->riff.Format), 1, fp);
    if (ret <= 0)
    {
        LOG_INFO("%s: read RIFF format fail\n", __func__);
        goto get_wav_info_fail;
    }

    if( memcmp(pinfo->riff.Format,"WAVE",4) && memcmp(pinfo->riff.Format,"wave",4) )
    {
        LOG_INFO("%s: %s is not a valid wav file\n",__func__,filename);
        goto get_wav_info_fail;
    }

    // read FMT
    while(1)
    {
        memset(&pinfo->fmt,0,sizeof(FMT_t));
        memset(rbuff,0,sizeof(rbuff));
        ret = ql_fread(pinfo->fmt.Subchunk1ID,sizeof(pinfo->fmt.Subchunk1ID),1,fp);
        if( ret <= 0 )
        {
            LOG_INFO("%s: read fmt id fail\n",__func__);
            goto get_wav_info_fail;
        }
        ret = ql_fread(rbuff, sizeof(uint32_t), 1, fp);
        if( ret <= 0 )
        {
            LOG_INFO("%s: read fmt len fail\n",__func__);
            goto get_wav_info_fail;
        }
        pinfo->fmt.Subchunk1Size = LITTLE_TO_U32(rbuff);

        if (memcmp(pinfo->fmt.Subchunk1ID, "fmt ", 4) || memcmp(pinfo->fmt.Subchunk1ID, "FMT ", 4))
        {
            // found FMT info
            break;
        }
        // other info,show them
        LOG_DBG("%s: found other info %.*s, size %ld\n", __func__, 4, pinfo->fmt.Subchunk1ID, pinfo->fmt.Subchunk1Size);
    }
    ret = ql_fread(rbuff,sizeof(uint16_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT format fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.AudioFormat = LITTLE_TO_U16(rbuff);
    if( pinfo->fmt.AudioFormat != 1 )
    {
        // not a pcm format
        LOG_INFO("%s: wav format is not pcm\n",__func__);
        goto get_wav_info_fail;
    }
    // read channel num
    ret = ql_fread(rbuff,sizeof(uint16_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT channel fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.NumChannels = LITTLE_TO_U16(rbuff);
    // read samplerate
    ret = ql_fread(rbuff,sizeof(uint32_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT samplerate fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.SampleRate = LITTLE_TO_U32(rbuff);
    // read byterate
    ret = ql_fread(rbuff,sizeof(uint32_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT byterate fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.ByteRate = LITTLE_TO_U32(rbuff);
     // read blockalign
    ret = ql_fread(rbuff,sizeof(uint16_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT blockalign fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.BlockAlign = LITTLE_TO_U16(rbuff);
    // read bitspersample
    ret = ql_fread(rbuff,sizeof(uint16_t),1,fp);
    if( ret <= 0 )
    {
        LOG_INFO("%s: read FMT bitspersample fail\n",__func__);
        goto get_wav_info_fail;
    }
    pinfo->fmt.BitsPerSample = LITTLE_TO_U16(rbuff);

    // normally,the FMT size is 16,but maybe include extract info,so skip the extra info
    if( pinfo->fmt.Subchunk1Size != 16 )
    {
        LOG_DBG("%s: skip %ld byte in FMT\n",__func__,pinfo->fmt.Subchunk1Size - 16);
        ql_fseek(fp,pinfo->fmt.Subchunk1Size - 16,SEEK_CUR);
    }

    // read DATA info
    while(1)
    {
        memset(&pinfo->data,0,sizeof(WavData_t));
        memset(rbuff,0,sizeof(rbuff));
        ret = ql_fread(pinfo->data.Subchunk2ID,sizeof(pinfo->data.Subchunk2ID),1,fp);
        if( ret <= 0 )
        {
            LOG_INFO("%s: read data id fail\n",__func__);
            goto get_wav_info_fail;
        }

        ret = ql_fread(rbuff, sizeof(uint32_t), 1, fp);
        if( ret <= 0 )
        {
            LOG_INFO("%s: read data len fail\n",__func__);
            goto get_wav_info_fail;
        }
        pinfo->data.Subchunk2Size = LITTLE_TO_U32(rbuff);

        if (memcmp(pinfo->data.Subchunk2ID, "data", 4) || memcmp(pinfo->fmt.Subchunk1ID, "DATA", 4))
        {
            // found DATA info
            pinfo->data.DataOffset = ql_ftell(fp);
            break;
        }
        // other info,show them
        LOG_DBG("%s: found other info %.*s, size %ld\n", __func__, 4, pinfo->data.Subchunk2ID, pinfo->data.Subchunk2Size);
    }
    ret = 0;
    goto get_wav_info_end;

    get_wav_info_fail:
    ret = -1;
    get_wav_info_end:
    ql_fclose(fp);

    if( 0 == ret )
    {
        LOG_DBG("-----------%s info------------\n",filename);
#if 0
        show_wav_info(pinfo);
#endif
    }
    return ret;
}

void wav_set_mix_duration(int duration_ms)
{
    LOG_INFO("%s: %d\n",__func__,duration_ms);
    wav_mix_duration = duration_ms;
}

int wav_mix_list(int size ,short const * group)
{
#define RBUFF_SIZE        1024
    int ret = 0;
    int mix_duration_ms = wav_mix_duration;
    int index = 0;
    char filename[64];
    int blocksize = 2;  // default to 16bit,2byte
    uint8_t *mixbuff = 0;
    uint32_t mixbuff_len = 0;
    uint32_t mixbuff_size = MIX_BUFF_SIZE;
    uint32_t offset = 0;
    int rsize = 0;
    uint8_t *rbuff = NULL; //rbuff[RBUFF_SIZE];
    MixCtrl_t mixc1,mixc2;
    uint8_t *pbuff;
    uint32_t timer_wait_finish = ql_rtos_get_systicks_to_ms();

    // TEST CODE START
    uint32_t timer_info_interval;
    uint32_t timer_total_cost = ql_rtos_get_systicks_to_ms();
    // TEST CODE END


    memset(&mixc1,0,sizeof(MixCtrl_t));
    memset(&mixc2,0,sizeof(MixCtrl_t));

    if( !size)
    {
        return -1;
    }
    memset(filename,0,sizeof(filename));
    snprintf(filename,sizeof(filename),"%s%s",WAV_FILE_PATH_PREFIX,GetRecInfo(group[index])->filePinyinName);

     rbuff = malloc(RBUFF_SIZE);
    if( !rbuff )
    {
        LOG_INFO( "%s: alloc rbuff fail\n", __func__ );
        return -1;
    }

    while(1)
    {
        wav_data_buff = calloc( 1, mixbuff_size );
//        goto wav_mix_list_ex_end;
        if ( !wav_data_buff )
        {
            mixbuff_size = mixbuff_size * 2 / 3;
            if( mixbuff_size <= (10 * 1024) )
            {
                LOG_INFO( "%s: alloc wav data buff fail\n", __func__ );
                //return -1;
                ret = -1;
                goto wav_mix_list_ex_end;
            }
            continue;
        }
        mixbuff_size /= 3;
        LOG_INFO("%s: mixbuff size %ld\n",__func__,mixbuff_size);
        break;
    }
    mixbuff = wav_data_buff;
    len_play_buff1 = 0;
    len_play_buff2 = 0;
    wav_play_buff1 = wav_data_buff + mixbuff_size;
    wav_play_buff2 = wav_data_buff + 2 * mixbuff_size;

    ret = get_wav_info(filename, &mixc1.info);
    if (ret)
    {
        LOG_INFO("%s: get %s info fail\n",__func__,filename);
        goto wav_mix_list_ex_end;
    }
    mixc1.fp = ql_fopen(filename,"rb");
    if( !mixc1.fp )
    {
        LOG_INFO("%s: open %s error\n",__func__,filename);
        goto wav_mix_list_ex_end;
    }
    else
    {
			LOG_INFO("%s_%d play \"%s\":\n", __func__, __LINE__,filename);
    }
    ql_fseek(mixc1.fp,mixc1.info.data.DataOffset,SEEK_SET);
    mixc1.start_offset = mixc1.info.data.Subchunk2Size - mixc1.info.fmt.ByteRate * mix_duration_ms/1000;
    if( mixc1.start_offset < 0 )
    {
        mixc1.start_offset = 0;
    }
    //blocksize = info.fmt.BitsPerSample/8;

    index++;
    if( index>=size )
    {
        goto wav_mix_list_ex_end;
    }
    memset(filename,0,sizeof(filename));
    snprintf(filename,sizeof(filename),"%s%s",WAV_FILE_PATH_PREFIX,GetRecInfo(group[index])->filePinyinName);

    ret = get_wav_info(filename, &mixc2.info);
    if (ret)
    {
        LOG_INFO("%s: get %s info fail\n",__func__,filename);
        goto wav_mix_list_ex_end;
    }
    mixc2.fp = ql_fopen(filename,"rb");
    if( !mixc2.fp )
    {
        LOG_INFO("%s: open %s error\n",__func__,filename);
        goto wav_mix_list_ex_end;
    }
    else
    {
			LOG_INFO("%s_%d play \"%s\":\n", __func__, __LINE__,filename);
    }
    	
    ql_fseek(mixc2.fp,mixc2.info.data.DataOffset,SEEK_SET);
    mixc2.start_offset = mixc2.info.data.Subchunk2Size - mixc2.info.fmt.ByteRate * mix_duration_ms / 1000;
    if (mixc2.start_offset < 0)
    {
        mixc2.start_offset = 0;
    }

    while(1)
    {
        //rsize = ql_fread(rbuff,1,blocksize,mixc1.fp);
        //if( rsize != blocksize )
        pbuff = mixbuff + mixbuff_len;
        rsize = ql_fread(pbuff,1,mixbuff_size - mixbuff_len,mixc1.fp);
        if( rsize <= 0 )
        {
            // read end or error,switch to next
            ql_fclose(mixc1.fp);
            mixc1.fp = 0;
            if( !mixc2.fp )
            {
                LOG_INFO("%s: all is done\n",__func__);
                goto wav_mix_list_ex_done;
            }
            // mixc2 become mixc1
            memcpy(&mixc1,&mixc2,sizeof(MixCtrl_t));
            memset(&mixc2,0,sizeof(MixCtrl_t));

            // set mixc2 to next file
            index++;
            if( index<size )
            {
                memset(filename,0,sizeof(filename));
                snprintf(filename,sizeof(filename),"%s%s",WAV_FILE_PATH_PREFIX,GetRecInfo(group[index])->filePinyinName);

                timer_info_interval = ql_rtos_get_systicks_to_ms();
                ret = get_wav_info(filename, &mixc2.info);
                LOG_DBG("%s: get info cost %ldms\n",__func__,ql_rtos_get_systicks_to_ms() - timer_info_interval);
                if (ret)
                {
                    LOG_INFO("%s: get %s info fail\n",__func__,filename);
                    goto wav_mix_list_ex_end;
                }
                mixc2.fp = ql_fopen(filename,"rb");
                if( !mixc2.fp )
                {
                    LOG_INFO("%s: open %s error\n",__func__,filename);
                    ret = -1;
                    goto wav_mix_list_ex_end;
                }
                else
                {
                	LOG_INFO("%s_%d play \"%s\":\n", __func__, __LINE__,filename);
                }
                ql_fseek(mixc2.fp,mixc2.info.data.DataOffset,SEEK_SET);
                mixc2.start_offset = mixc2.info.data.Subchunk2Size - mixc2.info.fmt.ByteRate * mix_duration_ms / 1000;
                if (mixc2.start_offset < 0)
                {
                    mixc2.start_offset = 0;
                }
            }
            else
            {
                LOG_INFO("%s: end of list\n",__func__);
            }
            continue;
        }

#if 1
        // optimize start
        offset = 0;
        if( mixc1.rcount < mixc1.start_offset)
        {
            if ( ( mixc1.start_offset - mixc1.rcount ) >= rsize )
            {
                offset = rsize / blocksize - 1;
            }
            else
            {
                offset = ( mixc1.start_offset - mixc1.rcount ) / blocksize - 1;
            }
        }
        mixc1.rcount += offset * blocksize;
        mixbuff_len += offset * blocksize;
        LOG_DBG("%s: rsize=%d,offset=%d,rcount=%ld,mlen=%ld,s_offset=%ld\n",__func__,rsize,offset,mixc1.rcount,mixbuff_len,mixc1.start_offset);
        // optimize end
#endif

        for(int i = offset,j = 0,rsize2 = 0; i < rsize/blocksize; i++ )
        {
            mixc1.voice_data = (pbuff[blocksize*i + 1] << 8) + pbuff[blocksize*i];
            mixc1.rcount += blocksize;

            // need to mix?
            if( (mixc1.rcount >= mixc1.start_offset) && mixc2.fp )
            {
                // has mix data ready?
                if( j >= rsize2 )
                {
                    // read data to mix
                    j = 0;
                    uint32_t remain_size = RBUFF_SIZE/blocksize;
                    if( remain_size > (rsize/blocksize - i) )
                    {
                        remain_size = rsize/blocksize - i;
                    }

                    rsize2 = ql_fread(rbuff,1,remain_size*blocksize,mixc2.fp);
                    if( rsize2 > 0 )
                    {
                        mixc2.rcount += rsize2;
                    }
                }

                if( (0 < rsize2) && (j < rsize2) )
                {
                    // start voice data mix
                    mixc2.voice_data = (rbuff[j+1] << 8) + rbuff[j];
                    j += blocksize;

                    mixc1.voice_data += mixc2.voice_data;
                }
            }

            mixbuff[mixbuff_len] = mixc1.voice_data;
            mixbuff[mixbuff_len + 1] = mixc1.voice_data >> 8;
            mixbuff_len += blocksize;

            if( (mixbuff_len >= mixbuff_size) )
            {
                // output data in buff
                do
                {
                    if( 0 == len_play_buff1 )
                    {
                        memcpy(wav_play_buff1,mixbuff,mixbuff_len);
                        len_play_buff1 = mixbuff_len;
                        LOG_DBG("%s: copy %ld bytes to buff1\n",__func__,mixbuff_len);
                        break;
                    }
                    else if( 0 == len_play_buff2 )
                    {
                        memcpy(wav_play_buff2,mixbuff,mixbuff_len);
                        len_play_buff2 = mixbuff_len;
                        LOG_DBG("%s: copy %ld bytes to buff2\n",__func__,mixbuff_len);
                        break;
                    }
                    // wait play end
                    ql_rtos_task_sleep_ms(10);
                }while(1);

                mixbuff_len = 0;
                //ql_rtos_task_sleep_ms(20);
            }
        }
    }

wav_mix_list_ex_done:
    if( mixbuff_len )
    {
        // output the remain mix data
        do
        {
            if ( 0 == len_play_buff1 )
            {
                memcpy( wav_play_buff1, mixbuff, mixbuff_len );
                len_play_buff1 = mixbuff_len;
                LOG_DBG( "%s: copy %ld bytes to buff1\n", __func__, mixbuff_len );
                break;
            }
            else if ( 0 == len_play_buff2 )
            {
                memcpy( wav_play_buff2, mixbuff, mixbuff_len );
                len_play_buff2 = mixbuff_len;
                LOG_DBG( "%s: copy %ld bytes to buff2\n", __func__, mixbuff_len );
                break;
            }
            // wait play end
            ql_rtos_task_sleep_ms( 10 );
        } while ( 1 );

        //memcpy(wav_data_buff,mixbuff,mixbuff_len);
        //wav_data_len = mixbuff_len;

        //wav_play_status = WAV_PLAY_PLAYING;
        mixbuff_len = 0;
    }

    // wait play finish
    timer_wait_finish = ql_rtos_get_systicks_to_ms() + 5*1000;
    while ( wav_play_status != WAV_PLAY_FINISH )
    {
        // wait play end
        if( !len_play_buff1 && !len_play_buff2 && (wav_play_status == WAV_PLAY_WAITING) )
        {
            // tell wav_play_task to stop and wait task end
            wav_play_status = WAV_PLAY_STOP;
        }
        if( ql_rtos_get_systicks_to_ms() > timer_wait_finish )
        {
            LOG_DBG("%s: wait play finish timeout,state=%d\n",__func__,wav_play_status);
            break;
        }
        ql_rtos_task_sleep_ms( 100 );
    }
    LOG_INFO("%s: play finish\n",__func__);

wav_mix_list_ex_end:
    // TEST CODE START
    LOG_DBG("%s: time cost %ldms\n",__func__,ql_rtos_get_systicks_to_ms() - timer_total_cost);
    // TEST CODE END

    if( mixc1.fp )
    {
        ql_fclose(mixc1.fp);
    }
    if( mixc2.fp )
    {
        ql_fclose(mixc2.fp);
    }
    if( wav_data_buff )
    {
        free(wav_data_buff);
        wav_data_len = 0;
        wav_data_buff = NULL;
    }
    if( rbuff )
    {
        free(rbuff);
    }

    return ret;
}

int wav_check_playlist(char **playlist)
{
    int ret;
    char buff[64];

    if( playlist == NULL )
    {
        return -1;
    }

    int index =0;
    while(1)
    {
        if( playlist[index] == NULL )
        {
            // playlist end with NULL
            break;
        }

        ret = check_wav_file(playlist[index],buff,sizeof(buff));
        if( ret )
        {
            LOG_INFO("%s: %s is not exist\n",__func__,playlist[index]);
            return -1;
        }
    }

    LOG_INFO("%s: playlist check ok\n",__func__);
    return 0;
}
int voice_pcm_write(void * handl,char const * pcmbuf,int buflen)
{
	int ii;
	int slen;
	while(buflen>0)
	{
		slen=buflen;
		if (slen>256) slen=256;
		if (ql_audio_track_write(handl, pcmbuf, slen)<0)
		{
			for(ii=0;ii<100;ii++)
			{
				if (ql_audio_track_write(handl, pcmbuf, slen)>=0)
					break;
					
				LOG_INFO("%s_%d ===pcm write fail len %d \n", __func__, __LINE__,slen);
				ql_rtos_task_sleep_ms( 10 );
			}
			if (ii>=100)
				return -1;
		}
		pcmbuf +=slen;
		buflen -=slen;
//		for(ii=0;ii<100;ii++)
//		{
//			if (ql_audio_track_write(handl, pcmbuf, buflen)>=0)
//				return 0;
//			
//			LOG_INFO("%s_%d ===pcm write fail len %d \n", __func__, __LINE__,buflen);
//			ql_rtos_task_sleep_ms( 10 );
//		}
		
	}
	
	return 0;
}

void wav_play_task(void *pvParameters)
{
    LOG_INFO("%s: start\n",__func__);

#if 1
	AUDIO_PLAY_OPTION option;
	option.option=0x100;
    option.latency2 = 4;
	option.write_mode=AUDIO_TRACK_WRITE_MODE_SYNC;
	option.mode=2;

    uint32_t timer_play_timeout = ql_rtos_get_systicks_to_s() + 10;
    uint32_t timer_wait_interval = ql_rtos_get_systicks_to_ms();
	audio_track_handle track_hd;
//	unsigned int channels = 2;
	unsigned int channels = 1;
	int write_ret=-1;

    //PCM_HANDLE_T pcm_write_hdl;
//    QL_PCM_CONFIG_T pcm_config = {1, WAV_SAMPLE_RATE, 0};
    openOffSpeaker(1);
//    pcm_write_hdl = ql_pcm_open(&pcm_config, QL_PCM_WRITE_FLAG|QL_PCM_BLOCK_FLAG);
//		track_hd = ql_audio_track_open(rate, channels, 0x100, NULL);
		track_hd = ql_audio_track_open(WAV_SAMPLE_RATE, channels, option.option, NULL);
		if (track_hd == NULL) {
			LOG_INFO("ql_pcm_open write fail\n");
		}
		else
    while(wav_play_status != WAV_PLAY_STOP)
    {
        if ( len_play_buff1 || len_play_buff2 )
        {
            LOG_INFO("%s: waiting interval %ldms\n",__func__,ql_rtos_get_systicks_to_ms() - timer_wait_interval);
            wav_play_status = WAV_PLAY_PLAYING;
            timer_play_timeout = ql_rtos_get_systicks_to_s( ) + 10;

            if ( len_play_buff1 )
            {
//                ql_pcm_write( pcm_write_hdl, wav_play_buff1, len_play_buff1 );
									write_ret=ql_audio_track_write(track_hd, wav_play_buff1, len_play_buff1);	
			LOG_INFO("%s_%d ===pcm write len %d ret:%d\n", __func__, __LINE__,len_play_buff1,write_ret);
//								if (voice_pcm_write(track_hd, wav_play_buff1, len_play_buff1)!=0)
//									break;
                len_play_buff1 = 0;
            }
            if ( len_play_buff2 )
            {
//                ql_pcm_write( pcm_write_hdl, wav_play_buff2, len_play_buff2 );
		write_ret=ql_audio_track_write(track_hd, wav_play_buff2, len_play_buff2);		  
			LOG_INFO("%s_%d ===pcm write len %d ret:%d\n", __func__, __LINE__,len_play_buff2,write_ret);
//								if (voice_pcm_write(track_hd, wav_play_buff1, len_play_buff1)!=0)
//									break;
                len_play_buff2 = 0;
            }

            LOG_DBG("%s: play over\n",__func__);
            // TEST CODE START
            timer_wait_interval = ql_rtos_get_systicks_to_ms( );
            // TEST CODE END
        }
        else
        {
            // idle
            wav_play_status = WAV_PLAY_WAITING;
            ql_rtos_task_sleep_ms( 10 );
        }

       if( ql_rtos_get_systicks_to_s() > timer_play_timeout )
       {
           break;
       }
    }

//    ql_pcm_close(pcm_write_hdl);
	ql_audio_track_close(track_hd, 0);
//    pcm_write_hdl = NULL;
    openOffSpeaker(0);
    wav_play_buff1 = NULL;
    wav_play_buff2 = NULL;
    len_play_buff1 = 0;
    len_play_buff2 = 0;
    wav_play_status = WAV_PLAY_FINISH;
    LOG_INFO("%s: end\n",__func__);
#endif
    wav_play_ctrl_task = NULL;
    ql_rtos_task_delete(NULL);
}


void wav_play_task_start(void)
{
    if( wav_play_ctrl_task )
    {
        // should not happen
        LOG_INFO("%s: wav play ctrl task is still running\n",__func__);
    }

    wav_play_status = WAV_PLAY_STOP;
    if (ql_rtos_task_create(&wav_play_ctrl_task,
                        5 * 1024,
                        97,
                        "wav_play_task",
                        wav_play_task,
                        NULL) != 0) {
        LOG_INFO("%s: thread create error\n",__func__);
    }
    wav_play_status = WAV_PLAY_WAITING;
}


int wav_mix_play_list(int size ,short const * group)
{
	wav_play_task_start();
	return wav_mix_list(size,group);
}


#endif




