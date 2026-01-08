#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include "ql_rtos.h"
#include "ql_audio.h"
#include "audiopack/fixaudio.h"
#include "pcm_play_num.h"
#include "res.h"
#include "tts_yt_task.h"
#include "voice_wav_mix.h"

#define LOG_DBG(...)            //do{printf("[DBG PCM]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(fmt, arg...) usb_log_printf("[TTS INFO]"fmt, ##arg)

int num_ttsstr_sub(int num,short * group,char * spkzero )
{
	short * pIdx;
	char tmp;
	
	if (num>=1000) return -1;
	if (num==0) 
	{
		if (*spkzero) *spkzero=2;
		return 0;
	}
	
	pIdx=group;
	
	tmp=num/100;
	num %=100;
	LOG_INFO("%s,line %d,num %d,mod %d,spkzero 0x%x\r\n",__func__,__LINE__,tmp,num,*spkzero);
	if (tmp)
	{
		if (*spkzero==2)
		{
			;//*ptr++=numchangascii[10];
		}
		*pIdx++=AUD_ID_NUM_0 +tmp;
		*pIdx++=AUD_ID_HUNDRED;
		*spkzero=1;
	}
	else if (*spkzero) *spkzero=2;
	
	tmp=num/10;
	num %=10;
	if (tmp)//
	{
		if (*spkzero==2)
		{
			;//*ptr++=numchangascii[10];
		}
		if(tmp==1)//ʮλΪһ
		{
			*pIdx++=AUD_ID_NUM_10 +num;
		}
		else
		{
			tmp-=2;
			*pIdx++=AUD_ID_NUM_20 +tmp;
			if(num)
			{
				*pIdx++=AUD_ID_NUM_0 +num;
			}
		}

	}
	else if(num)//���ʮλΪ0,�Ҹ�λ��Ϊ0
	{
			if (*spkzero==2)
			{
				;//*ptr++=numchangascii[10];;
			}
			*pIdx++=AUD_ID_NUM_0 +num;
	}

	return (pIdx-group);
}


int num_to_audio_idx(unsigned long num,short * group,int size)//??��??
{
	int dot;
	unsigned long tmp;
	short * pIdx;
	char spkzero=0;
	
	dot=num%100;
	num /=100;
	if(num>1000000)return -1;//
//	memset(tts_str,0,sizeof(tts_str));
	LOG_INFO("%s,line %d,dot %lu,num %lu\r\n",__func__,__LINE__,dot,num);

	pIdx=group;
	if (num)
	{
		tmp=num/1000000l;
		num %=1000000l;
		LOG_INFO("%s,line %d,num %lu,mod %lu\r\n",__func__,__LINE__,tmp,num);
		if (tmp)
		{
			pIdx +=num_ttsstr_sub(tmp,pIdx,&spkzero);
			*pIdx++=AUD_ID_MILLION;
			spkzero=1;
		}
		else if (spkzero) spkzero=2;

		tmp=num/1000l;
		num %=1000l;
		LOG_INFO("%s,line %d,num %lu,mod %lu\r\n",__func__,__LINE__,tmp,num);
		if (tmp)
		{
			pIdx +=num_ttsstr_sub(tmp,pIdx,&spkzero);
			*pIdx++=AUD_ID_THOUSAND;
			spkzero=1;
		}
		else if (spkzero) spkzero=2;

		if (num)
		{
			pIdx +=num_ttsstr_sub(num,pIdx,&spkzero);
		}
	}
	else
	{
		*pIdx++=AUD_ID_NUM_0;
	}
	
	LOG_INFO("%s,line %d,num %lu,mod %lu\r\n",__func__,__LINE__,tmp,num);
	if (dot)
	{
		*pIdx++=AUD_ID_DOT;
		num=dot/10;
		*pIdx++=AUD_ID_NUM_0 +num;
		num=dot%10;
		if (num)
		{
			*pIdx++=AUD_ID_NUM_0 +num;
		}
	}
	
	return (pIdx-group);
}


int numstr_to_audio_idx(const char * tts_str,short * pIdx,int size)
{
	double num;
	char const *tts_ptr = NULL;

	tts_ptr=tts_str;
	while(*tts_ptr)
	{
		char tmp;
		tmp=*tts_ptr++;
		if (((tmp<'0')||(tmp>'9'))&&(tmp!='.')) return -1;
	}

	tts_ptr = strchr(tts_str, '.');
	if (tts_ptr!=NULL) 
	{
		tts_ptr++;
		if (strchr(tts_ptr,'.')!=NULL) return -1;
	}
		
	num=0;
	sscanf(tts_str,"%lf",&num);
	// num *=100;

	return num_to_audio_idx((unsigned long )(num+0.5),pIdx,size);
}

