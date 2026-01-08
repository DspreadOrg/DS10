#ifndef __APP_TTS_H__
#define __APP_TTS_H__

//extern int num_ttsstr(unsigned long num,int dot,char * tts_str,int olen);
//extern int numstr_ttsstr(const char * tts_str,char *tts_buf,int olen);
//extern int ym_play_tts_str_audio(const char * numstr);
//extern int ym_play_tts_str_audio_ex(const char * tts_str);
//extern int ym_play_num_audio(const char * numstr,uint8_t mode);
//extern int ym_add_num_to_playlist(char *playlist,const char * numstr,uint8_t mode);
extern int numstr_to_audio_idx(const char * tts_str,short * pIdx,int size);
extern int num_to_audio_idx(unsigned long num,short * group,int size);

#endif

