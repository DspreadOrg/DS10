#ifndef __RECORD_PLAY_H__
#define __RECORD_PLAY_H__

typedef struct{
	uint32_t timeout_count;
	uint8_t play_record_count;
}Play_Record_Struct;

extern Play_Record_Struct play_history_record;

void Enter_Play_Record_Mode(void);
void Exit_Play_Record_Mode(void);
void Play_Record_Chk(void);
int Play_Record_Func(uint16_t count); 
int display_record_func(uint16_t count); 

#endif
