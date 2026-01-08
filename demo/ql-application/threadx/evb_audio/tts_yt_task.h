#ifndef _YT_TTS_TASK_H_
#define _YT_TTS_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "fixaudio.h"


#define QL_PCM_BLOCK_FLAG       (0x01)
#define QL_PCM_NONBLOCK_FLAG    (0x02)
#define QL_PCM_READ_FLAG        (0x04)
#define QL_PCM_WRITE_FLAG       (0x08)

#define TTS_FLAG_BIZCODE        0x03
#define TTS_FLAG_PAYMSG         0x04
#define TTS_FLAG_CANCEL         0x08
#define TTS_FLAG_RECORD         0x10
#define TTS_FLAG_FLASH          0x20


typedef void (* ttsPlayCB)();

typedef struct {
	uint32 timestamp;
	int64_t Amount;
	char * msgid;
	char * url;
	union{
		char * msg;
		uint32 SalePlayId;
	};
	char sbuf[1];
//	uint8 url[1];
}SaleInfo;

typedef struct {
	uint32 size;
	short Idx[1];
//	uint8 url[1];
}AudioGroup;

struct AudioPlayMsg{
	int Idx;	//消息索引
	int BreakEna;	//播报打断允许
//	int Insert;
	void * priv;//自定义，交易消息等，消息索引为特定值时处理
} ;

extern int TTsPlayRecIdx(	int recIdx);
extern void AuidoVolumeSet(int CurSet);
extern void tts_play_init(void);
extern int tts_play_set_idx(int idx, int BrkEna,int playnow);
extern int tts_play_set_group(short const * Group,int size,int BreakEna,int playnow );
extern int tts_play_set_sale_id(const char * msgid,const char * url,int SalePlayId,uint64_t Amount,uint32_t timestamp);
extern int tts_play_set_sale_str(const char * msgid,const char * url,char const * salemsg,uint64_t Amount,uint32_t timestamp);
//extern int tts_play_text(char* text_str, tts_text_type type);
//void tts_play_set(const char * text_str,int len, tts_text_type type);
//extern int tts_play_set(const char* buf,int len, tts_text_type type);
//extern int tts_play_immediately(const char* buf,int len, tts_text_type type);
extern int Module_TTS_Play_Factory_Text(char * tts_buf,void (*Switch)(uint8_t OnOff));
/***********************************************
WaitAudioPlayOver，
若发送语音消息后即调用，请先延迟500ms以上再调用
确保已发送语音消息已进入播放状态
语音播放结束，可能要一秒左右才会退出。
***********************************************/
extern void WaitAudioPlayOver(void);
extern void AudioPlayHalt(void);
extern void AudioPlayContinue(void);

#ifdef __cplusplus
}
#endif

#endif /* _AD_BUTTON_TASK_H_ */