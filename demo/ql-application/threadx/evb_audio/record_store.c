#include <stdio.h>
#include <stdlib.h>
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_rtc.h"
#include "public_api_interface.h"
#include "prj_common.h"
#include "record_store.h"
#include "cjson.h"

#if CFG_DBG
#define LOG_DBG(fmt, arg...) usb_log_printf("[RECORD DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) usb_log_printf("[RECORD INFO]"fmt, ##arg)
#else
#define LOG_DBG(fmt, arg...) ((void)0)
#define LOG_INFO(fmt, arg...) ((void)0)
#endif

#define RECORD_STORE_DEBUG      LOG_INFO
#define RECORD_STORE_ERROR      LOG_INFO

// ��ѯ���еļ�¼  ��ѯ�м�����¼
int Record_Get_Count(void)
{
    QFILE * fp = NULL;
    int ret = -1;
	int i=0;
	int cnt = 0;
	char *read_buf = NULL; //[RECORD_PAY_PER_LENGTH] = {0};

    read_buf = calloc(RECORD_PAY_PER_LENGTH,1);
    if( read_buf == NULL )
    {
        RECORD_STORE_DEBUG("%s: no enough mem\n",__func__);
        return -1;
    }

    fp = ql_fopen(RECORD_U_FILE_ROOT, "rb");
	if (fp == NULL) {
		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
        cnt = -1;
		goto exit;
	}
    
	for(i=0; i<RECORD_PAY_MAX_CNT; i++)
	{
        memset(read_buf, 0, RECORD_PAY_PER_LENGTH);
        ret = ql_fread((void *)read_buf, RECORD_PAY_PER_LENGTH, 1, fp);
        if ((ret < 0) || (ret != RECORD_PAY_PER_LENGTH))
    	{
    		RECORD_STORE_DEBUG("[FS] *** file read end: %s ***\r\n", RECORD_U_FILE_ROOT);
            if (ret != 0)
            {
                cnt = -1;
            }
		    goto exit;
    	}
        
		if((read_buf[0]==0xBB)&&(read_buf[1]==0xDD))
			cnt++;
		else
			break;
	}

exit:
	if(fp)
	{
		ql_fclose(fp);
		RECORD_STORE_DEBUG("%s_%d, file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
	}
	if( read_buf )
	{
	    free(read_buf);
	}
    
	return cnt;
}

// ��ȡ�Ѿ��м�����¼
int Record_Read_MSGID(uint32_t Addr)
{
    QFILE * fp = NULL;
    int ret = -1;
	uint8_t i=0; 
	char *read_buf = NULL; //[RECORD_PAY_PER_LENGTH];
	int record_count = 0;
	char *ptr = NULL;
	
	RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);

    read_buf = calloc(RECORD_PAY_PER_LENGTH,1);
    if( read_buf == NULL )
    {
        RECORD_STORE_DEBUG("%s: no enough mem\n",__func__);
        return -1;
    }

    fp = ql_fopen(RECORD_U_FILE_ROOT, "rb+");
	if (fp == NULL) {
		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
		record_count = -1;
		goto exit;
	}
    
	for(i=0; i<RECORD_PAY_MAX_CNT; i++)
	{
		memset(read_buf, 0, RECORD_PAY_PER_LENGTH);
        ret = ql_fread((void *)read_buf, RECORD_PAY_PER_LENGTH, 1, fp);
        if ((ret < 0) || (ret != RECORD_PAY_PER_LENGTH))
    	{
    		RECORD_STORE_DEBUG("[FS] *** file read fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            record_count = -1;
		    goto exit;
    	}

		if((read_buf[0] == 0xBB)&&(read_buf[1] == 0xDD))
		{
			record_count++;
		}
		else if((read_buf[0] == 0xff)&&(read_buf[1] == 0xff))
		{
			break;
		}
		else
		{
			if(record_count==0)
			{
				ql_ftruncate(fp, 0); // clear all data
			}
			else
			{
				ptr = malloc(RECORD_PAY_PER_LENGTH*record_count);
				if(ptr==NULL)
				{
					ql_ftruncate(fp, 0); // clear all data
					record_count = 0;
					break;
				}
                memset(ptr, 0, RECORD_PAY_PER_LENGTH*record_count);
				
                ql_fseek(fp, 0, SEEK_SET);
                ret = ql_fread((void *)ptr, RECORD_PAY_PER_LENGTH*record_count, 1, fp);
                if ((ret < 0) || ((ret < 0) || (ret != RECORD_PAY_PER_LENGTH*record_count)))
            	{
            		RECORD_STORE_DEBUG("[FS] *** file read fail: %s ***\r\n", RECORD_U_FILE_ROOT);
                    free(ptr);
                    record_count = -1;
		            goto exit;
            	}
				ql_ftruncate(fp, 0); // clear all data

                ql_fseek(fp, 0, SEEK_SET);
                ret = ql_fwrite((void *)ptr, RECORD_PAY_PER_LENGTH*record_count, 1, fp);
            	if(ret < 0) 
            	{
            		RECORD_STORE_ERROR("[FS] *** file write fail: %s ***\r\n", RECORD_U_FILE_ROOT);
                    free(ptr);
                    record_count = -1;
		            goto exit;
            	}
				free(ptr);
			}
			break;			
		}
	}

exit:
	if(fp)
	{
		ql_fclose(fp);
		RECORD_STORE_DEBUG("%s_%d, file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
	}
	if( read_buf )
	{
	    free(read_buf);
	}
    
	return record_count;
}


// �Ա�msgid�Ƿ����ظ�
int Record_Compare_MSGID(uint32_t Addr, char * msgid)
{
    QFILE * fp = NULL;
	uint8_t i=0; 
	cJSON*root = NULL;
	cJSON*psub = NULL;
	uint16_t len = 0;
	char *read_buf = NULL; //[RECORD_PAY_PER_LENGTH];
	char *cjson_buf = NULL; //[RECORD_PAY_PER_LENGTH];
	int ret = 0;
	RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);

	read_buf = calloc(2 * RECORD_PAY_PER_LENGTH,1);
	if( read_buf == NULL )
	{
        RECORD_STORE_DEBUG("%s: no enough mem\n",__func__);
        return -1;
	}
	cjson_buf = read_buf + RECORD_PAY_PER_LENGTH;

    fp = ql_fopen(RECORD_U_FILE_ROOT, "rb");
	if (fp == NULL) {
		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
		ret = -1;
		goto exit;
	}
    
	for(i=0; i<RECORD_PAY_MAX_CNT; i++)
	{
		memset(read_buf, 0, RECORD_PAY_PER_LENGTH);
        ret = ql_fread((void *)read_buf, RECORD_PAY_PER_LENGTH, 1, fp);
        if ((ret < 0) || (ret != RECORD_PAY_PER_LENGTH))
    	{
    		RECORD_STORE_DEBUG("[FS] *** file read fail: %s ***, ret = %d\r\n", RECORD_U_FILE_ROOT, ret);
            if (ret == 0)
            {
                ret = 0;
            }
            else
            {
                ret = -1;
            }
		    goto exit;
    	}
        ret = 0;

		if((read_buf[0] == 0xBB)&&(read_buf[1] == 0xDD))
		{
			len = read_buf[2];
			len = (len<<8)+read_buf[3];
			memset(cjson_buf, 0, RECORD_PAY_PER_LENGTH);
			memcpy(cjson_buf, &read_buf[4], len);
			
			root=cJSON_Parse((void *)cjson_buf);
			psub = cJSON_GetObjectItem(root,"MsgID");
			if ((NULL==psub)||(psub->type!=cJSON_String))
			{
				if (root != NULL)
				{
					cJSON_Delete(root);
					root = NULL;
				}
				continue;
			}
			
			RECORD_STORE_DEBUG("new messageId = %s\n", msgid);
			RECORD_STORE_DEBUG("%d read messageId = %s\n", i, psub->valuestring);
			
			if(!strcmp(msgid, psub->valuestring))
			{
				if (root != NULL)
				{
					cJSON_Delete(root);
					root = NULL;
				}
				ret = -1;
				break;
			}
			
			cJSON_Delete(root);
			root = NULL;
		}
		else if((read_buf[0] == 0xff)&&(read_buf[1] == 0xff))
		{
			break;
		}
		else
		{
			//ret = -1;
			ql_ftruncate(fp, 0); // clear all data
			break;			
		}
	}

exit:
	if(fp)
	{
		ql_fclose(fp);
		RECORD_STORE_DEBUG("%s_%d, file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
	}
	if( read_buf )
	{
	    free(read_buf);
	}
    
	return ret;
}

// ������в�����¼
int Record_Manage_Clean(void)
{
    QFILE * fp = NULL;

    fp = ql_fopen(RECORD_U_FILE_ROOT, "wb+");
	if (fp == NULL) {
		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
		return -1;
	}

    if (fp != NULL)
    {
        ql_fclose(fp);
    }

    return 0;
}



// ���� �洢������¼
int Record_Manage_Write(char * msgid, char* voiceMsg, int64_t money)
{
    QFILE * fp = NULL;
    int ret = -1;
	cJSON *root = NULL;
	char * out = NULL;
	char *record_buf = NULL;  //[RECORD_PAY_PER_LENGTH] = {0xBB, 0xDD};
	uint8_t cjson_len = 0;
	int record_cnt = 0;
	char *record_history_buf = NULL;
	uint32_t timestamp = read_rtc_time();
	//char read_buf[RECORD_PAY_PER_LENGTH] = {0};

	record_buf = malloc(RECORD_PAY_PER_LENGTH);
	if( record_buf == NULL )
	{
	    RECORD_STORE_DEBUG("%s: no enough mem\n",__func__);
	    return -1;
	}
    memset(record_buf, 0xFF, RECORD_PAY_PER_LENGTH);
    record_buf[0] = 0xBB;
    record_buf[1] = 0xDD;
	RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);
	root = cJSON_CreateObject();
	if(!root)
	{
	    ret = -1;
	    goto exit;
	}
	cJSON_AddItemToObject(root, "TimeStamp", cJSON_CreateNumber(timestamp));
	cJSON_AddItemToObject(root, "MsgID", cJSON_CreateString(msgid));
	cJSON_AddItemToObject(root, "Money", cJSON_CreateNumber(money));
	cJSON_AddItemToObject(root, "VoiceMsg", cJSON_CreateString(voiceMsg));
	out=cJSON_Print(root);
	cJSON_Delete(root);
	cjson_len = strlen(out);
	if( cjson_len > RECORD_PAY_PER_LENGTH-8 )
	{
	    printf("%s: msg record is too long,drop\n",__func__);
	    free(out);
	    ret = -1;
	    goto exit;
	}
	memcpy(&record_buf[4], out, cjson_len);
	free(out);

	record_buf[2] = cjson_len>>8;
	record_buf[3] = cjson_len&0xff;
	record_buf[cjson_len+4] = 0xDD;
	record_buf[cjson_len+5] = 0xBB;
	
	record_cnt = Record_Get_Count();
	RECORD_STORE_DEBUG("enter %s ,line %d, record_cnt:%d\n", __func__, __LINE__, record_cnt);
	if(record_cnt<RECORD_PAY_MAX_CNT)
	{
        RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);
		if(record_cnt == 0)
		{
			fp = ql_fopen(RECORD_U_FILE_ROOT, "wb+");
        	if (fp == NULL) {
        		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
        		goto exit;
        	}	
		}
        else
        {
			fp = ql_fopen(RECORD_U_FILE_ROOT, "ab+");
        	if (fp == NULL) {
        		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
        		goto exit;
        	}	
        }
	    RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);
        ql_fseek(fp, record_cnt*RECORD_PAY_PER_LENGTH, SEEK_SET);
		ret = ql_fwrite((void *)record_buf, RECORD_PAY_PER_LENGTH, 1, fp);
    	if(ret < 0) 
    	{
    		RECORD_STORE_ERROR("[FS] *** file write fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            goto exit;
    	}

	}
	else
	{
		RECORD_STORE_DEBUG("enter %s ,line %d, start backup history data\n", __func__, __LINE__);
		//char record_history_buf[RECORD_HISTORY_BUF_LEN] = {0};
		record_history_buf = calloc(RECORD_HISTORY_BUF_LEN,1);
		if( record_history_buf == NULL )
		{
		    printf("%s: not enough mem\n",__func__);
		    goto exit;
		}

        fp = ql_fopen(RECORD_U_FILE_ROOT, "ab+");
    	if (fp == NULL) {
    		RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
    		goto exit;
    	}
        ql_fseek(fp, RECORD_PAY_PER_LENGTH*RECORD_PAY_BACKUP_START, SEEK_SET);
        memset(record_history_buf, 0, RECORD_HISTORY_BUF_LEN);
        ret = ql_fread((void *)record_history_buf, RECORD_HISTORY_BUF_LEN, 1, fp);
        if ((ret < 0) || (ret != RECORD_HISTORY_BUF_LEN)) 
    	{
    		RECORD_STORE_DEBUG("[FS] *** file read fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            goto exit;
    	}
#if 0
        RECORD_STORE_DEBUG("%s: truncate file\n",__func__);
        ql_ftruncate(fp, 0);

        ql_fseek(fp, 0, SEEK_SET);
#else
        ql_fclose(fp);
        fp = ql_fopen( RECORD_U_FILE_ROOT, "wb+" );
        if ( fp == NULL )
        {
            RECORD_STORE_ERROR("%s_%d === reopen %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
            goto exit;
        }
#endif
		ret = ql_fwrite((void *)record_history_buf, RECORD_HISTORY_BUF_LEN, 1, fp);
    	if(ret < 0) 
    	{
    		RECORD_STORE_ERROR("[FS] *** file write fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            goto exit;
    	}

		ret = ql_fwrite((void *)record_buf, RECORD_PAY_PER_LENGTH, 1, fp);
    	if(ret < 0) 
    	{
    		RECORD_STORE_ERROR("[FS] *** file write fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            goto exit;
    	}
	}

exit:
	if(fp)
	{
		ql_fclose(fp);
		RECORD_STORE_DEBUG("%s_%d, file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
	}
	if( record_history_buf )
	{
	    free(record_history_buf);
	}
	if( record_buf )
	{
	    free(record_buf);
	}
    
	return ret;
}

// ��ȡĳ��������¼
int Record_Manage_Read(uint16_t read_cnt, char* playbuf, uint8_t * play_len)
{
    QFILE * fp = NULL;
    int ret = -1;
	char *record_buf = NULL; //[RECORD_PAY_PER_LENGTH] = {0};
	int record_cnt = 0;
	uint16_t len = 0;

	*play_len = 0;
    record_buf = malloc(RECORD_PAY_PER_LENGTH);
    if( record_buf == NULL )
    {
        RECORD_STORE_DEBUG("%s: no enough mem\n",__func__);
        return -1;
    }
    memset(record_buf,0,RECORD_PAY_PER_LENGTH);
    
	RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);
	record_cnt = Record_Get_Count();
	RECORD_STORE_DEBUG("enter %s ,line %d, read_cnt:%d, record_cnt:%d\n", __func__, __LINE__, read_cnt, record_cnt);

    RECORD_STORE_DEBUG("enter %s ,line %d\n", __func__, __LINE__);
    if(read_cnt<=record_cnt)
    {
        fp = ql_fopen(RECORD_U_FILE_ROOT, "rb");
        if (fp == NULL) {
            RECORD_STORE_ERROR("%s_%d === open %s file failed!\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
            goto exit;
        }
        ql_fseek(fp, (record_cnt-read_cnt)*RECORD_PAY_PER_LENGTH, SEEK_SET);
        memset(record_buf, 0, RECORD_PAY_PER_LENGTH);
        ret = ql_fread((void *)record_buf, RECORD_PAY_PER_LENGTH, 1, fp);
        if ((ret < 0) || (ret != RECORD_PAY_PER_LENGTH))
        {
            RECORD_STORE_DEBUG("[FS] *** file read fail: %s ***\r\n", RECORD_U_FILE_ROOT);
            goto exit;
        }

        len = record_buf[2];
        len = (len<<8)+record_buf[3];
        if( len > RECORD_PAY_PER_LENGTH )
        {
            len = RECORD_PAY_PER_LENGTH;
        }
        memcpy(playbuf, &record_buf[4], len);
        *play_len = len;
        RECORD_STORE_DEBUG("enter %s ,line %d, len:%d\n", __func__, __LINE__, len);
    }
    else
    {
        *play_len = 0;
        ret = 0;
        goto exit;
    }


exit:
	if(fp)
	{
		ql_fclose(fp);
		RECORD_STORE_DEBUG("%s_%d, file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
	}
	if( record_buf )
    {
        free(record_buf);
    }
    
	return ret;
}

int Record_Manage_Init(void)
{
    int ret = -1;
    QFILE * fp = NULL;
    
	ret = ql_access(RECORD_U_FILE_ROOT, 0);

    if (ret != LFS_ERR_OK) // file not exist
    {
    	fp = ql_fopen(RECORD_U_FILE_ROOT, "w+");
    	if(fp == NULL)
    	{
    		RECORD_STORE_ERROR("%s_%d === *** file create fail: %s ***\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
    		return -1;
    	}

    	if(fp)
    	{
    		ql_fclose(fp);
    		RECORD_STORE_ERROR("%s_%d ===[FS] file closed: %s\r\n", __func__, __LINE__, RECORD_U_FILE_ROOT);
    	}
    }

    return ret;
}


