#ifndef __RECORD_STORE_H__
#define __RECORD_STORE_H__

#define RECORD_PAY_MAX_CNT          100
#define RECORD_PAY_BACKUP_START     90
#define RECORD_PAY_BACKUP_CNT       (RECORD_PAY_MAX_CNT-RECORD_PAY_BACKUP_START)
#define RECORD_PAY_PER_LENGTH       200
#define RECORD_HISTORY_BUF_LEN      ((RECORD_PAY_MAX_CNT-RECORD_PAY_BACKUP_START)*RECORD_PAY_PER_LENGTH)

#define RECORD_U_FILE_ROOT    "U:/pay_record.dat"     //save record data

int Record_Get_Count(void);
int Record_Manage_Write(char * msgid, char* voiceMsg, int64_t money);
//int Record_Manage_Write(char * msgid, char* voiceMsg);
int Record_Manage_Read(uint16_t read_cnt, char* playbuf, uint8_t * play_len);
int Record_Compare_MSGID(uint32_t Addr, char * msgid);
int Record_Manage_Clean(void);
int Record_Read_MSGID(uint32_t Addr);
int Record_Manage_Init(void);

#endif
