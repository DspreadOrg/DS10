/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/


/*===========================================================================
 * include files
 ===========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_application.h"
#include "ql_zip.h"
/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_ZIPDEMO_LOG(fmt, args...)	printf("[%s %d]"fmt,__func__,__LINE__, ##args)

#define FILE_NAME1			"testFile1.txt"
#define FILE_NAME2			"testFile2.txt"
#define FILE_ZIP_NAME		"test.zip"
#define FILE_NAME_ZIP1		"test01.txt"
#define FILE_NAME_ZIP2		"test02.txt"
#define U_DISK				"U:"
#define U_FILE_UNDER_ROOT1 	""U_DISK"/"FILE_NAME1""
#define U_FILE_UNDER_ROOT2 	""U_DISK"/"FILE_NAME2""
#define U_FILE_UNDER_ZIP1 	""U_DISK"/"FILE_NAME_ZIP1""
#define U_FILE_UNDER_ZIP2 	""U_DISK"/"FILE_NAME_ZIP2""
#define U_FILE_UNDER_ZIP 	""U_DISK"/"FILE_ZIP_NAME""
#define DIR_NAME			"testDir"
#define U_DIR_PATH 			""U_DISK"/"DIR_NAME""


#define ZIP_FILE_LEN    (1024)
/*===========================================================================
 * Variate
 ===========================================================================*/
const char *unZipFilePath=U_FILE_UNDER_ZIP;

char *unZip_path=NULL;
const char *TestFilePath[2]={U_FILE_UNDER_ROOT1,U_FILE_UNDER_ROOT2};
const char *TestFilezipname[2]={U_FILE_UNDER_ZIP1,U_FILE_UNDER_ZIP2};
/*===========================================================================
 * Functions
 ===========================================================================*/
static void ql_zip_demo_thread(void *param)
{
 	int ret = 0;
    int run_num=0;
    HZIP hz=0;
    QFILE * fp1 = NULL;
	QFILE * fp2 = NULL;
    unsigned char *zip_fs_space=calloc(10,ZIP_FILE_LEN);
    if(NULL==zip_fs_space)
    {
        QL_ZIPDEMO_LOG("malloc err\r\n");
        return; 
    }
	ql_rtos_task_sleep_s(5);
    QL_ZIPDEMO_LOG("========== zip demo start ==========\r\n");
    QL_ZIPDEMO_LOG("========== start create Test Files:[%s][%s]==========\r\n",TestFilePath[0],TestFilePath[1]);
    fp1 = ql_fopen(TestFilePath[0],"w+");
    fp2 = ql_fopen(TestFilePath[1],"w+");
    if((fp1 == NULL)||(fp2 == NULL))
    {
        QL_ZIPDEMO_LOG("open file failed ! %p-%p\r\n",fp1,fp2);
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] fail==========\r\n",TestFilePath[0],TestFilePath[1]);
        goto main_exit;
    }
    else 
    {
        QL_ZIPDEMO_LOG("open file:[%x][%x] success!\r\n",fp1,fp2);
    }
    memset(zip_fs_space,'a',10*ZIP_FILE_LEN);
    ret = ql_fwrite(zip_fs_space, 10*ZIP_FILE_LEN, 1, fp1);
    if(ret > 0)
	{
		QL_ZIPDEMO_LOG("file:test01.txt write :%d bytes data\r\n", ret);
	}
    else
    {
        ql_fclose(fp1);
        ql_fclose(fp2);
        QL_ZIPDEMO_LOG("file:test01.txt write Failed\r\n");
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] Failed==========\r\n",TestFilePath[0],TestFilePath[1]);
        goto main_exit;
    }
    memset(zip_fs_space,'b',5*ZIP_FILE_LEN);
    ret = ql_fwrite(zip_fs_space, 5*ZIP_FILE_LEN, 1, fp2);
    if(ret > 0)
	{
		QL_ZIPDEMO_LOG("file:test02.txt write :%d bytes data\r\n", ret);
	}
    else
    {
        ql_fclose(fp1);
        ql_fclose(fp2);
        QL_ZIPDEMO_LOG("file:test02.txt write Failed\r\n");
        QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] Failed==========\r\n",TestFilePath[0],TestFilePath[1]);
        goto main_exit;
    }
	ql_fclose(fp1);
	ql_fclose(fp2);
    QL_ZIPDEMO_LOG("========== Create Test Files:[%s][%s] success==========\r\n",TestFilePath[0],TestFilePath[1]);
	ql_rtos_task_sleep_s(5); 
    
    for(run_num=0;run_num<1;run_num++)
    {
          QL_ZIPDEMO_LOG("========== loop_test[%d] start==========\r\n",run_num+1);
          QL_ZIPDEMO_LOG("========== start zip file[%d]==========\r\n",run_num+1);
          
          hz = ql_fs_zip_create(unZipFilePath,0,&ret);
          if (ret<0)
          {
               QL_ZIPDEMO_LOG("create empty.zip:[%s] Failed,errcode=%d\r\n",unZipFilePath,ret);
               goto main_exit;
          }
          else
          {
               QL_ZIPDEMO_LOG("create empty.zip:[%s] Success\r\n",unZipFilePath);
          }
         
          ret=ql_fs_zip_add_file(hz,TestFilezipname[0],TestFilePath[0]); 
          if (ret!=QL_FILE_OK) 
          {
               QL_ZIPDEMO_LOG("add [%s] to zip Failed\r\n",TestFilePath[0]);
               goto main_exit;
          }
          QL_ZIPDEMO_LOG("add [%s] to zip Success\r\n",TestFilePath[0]);

          ret=ql_fs_zip_add_file(hz,TestFilezipname[1],TestFilePath[1]); 
          if (ret!=QL_FILE_OK) 
          {
               QL_ZIPDEMO_LOG("add [%s] to zip Failed\r\n",TestFilePath[1]);
               goto main_exit;
          }
          QL_ZIPDEMO_LOG("add [%s] to zip Success\r\n",TestFilePath[1]);

          ql_fs_zip_close(hz);   
#if 0
          //?????
          ql_rtos_task_sleep_s(5);
          ql_unzip_mem_data_s *ql_unzip_tmp = (ql_unzip_mem_data_s *)calloc(1, sizeof(ql_unzip_mem_data_s));
          if (ql_unzip_tmp == NULL)
          {
                QL_ZIPDEMO_LOG("malloc err");
                goto main_exit;
          }
          
          ql_unzip_tmp->data = (void *)calloc(1, 15*ZIP_FILE_LEN);
          if (ql_unzip_tmp->data == NULL)
          {
                QL_ZIPDEMO_LOG("malloc err");
                free(ql_unzip_tmp);
                goto main_exit;
          }

          ql_unzip_tmp->total_len = 15*ZIP_FILE_LEN;
          ret = ql_fs_unzip_ex(unZipFilePath, QL_UNZIP_TO_MEMERY, ql_unzip_tmp);
          if(ret!=QL_FILE_OK)
          {
                QL_ZIPDEMO_LOG("unzip file:[%s] Failed",unZipFilePath);
                free(ql_unzip_tmp->data);
                free(ql_unzip_tmp);
                goto main_exit;
          }
          QL_ZIPDEMO_LOG("used_len=%d,item_num=%d\n",ql_unzip_tmp->used_len, ql_unzip_tmp->item_num);
          
          int j=0;
          int i = 0;
          char *tmep = NULL;
          for (j=0; j<ql_unzip_tmp->item_num; j++)
          {
              QL_ZIPDEMO_LOG("file path=%s\n", ql_unzip_tmp->item[j].file_path);
              QL_ZIPDEMO_LOG("file len=%d,type=%d\n", ql_unzip_tmp->item[j].item_len, ql_unzip_tmp->item[j].file_type);
              tmep  = (char *)ql_unzip_tmp->item[j].item_data;
              if (tmep && ql_unzip_tmp->item[j].file_type == QL_UNZIP_FILE)
              {
                    if (0 == strcmp(ql_unzip_tmp->item[j].file_path, TestFilePath[0]))
                    {
                        for (i=0; i<ql_unzip_tmp->item[j].item_len; i++)
                        {
                             if (tmep[i] != 'a')
                             {
                                  QL_ZIPDEMO_LOG("not match -----------");
                                  break;
                             }
                        }
                    }

                    if (0 == strcmp(ql_unzip_tmp->item[j].file_path, TestFilePath[1]))
                    {
                        for (i=0; i<ql_unzip_tmp->item[j].item_len; i++)
                        {
                             if (tmep[i] != 'b')
                             {
                                  QL_ZIPDEMO_LOG("not match -----------");
                                  break;
                             }
                        }
                    }    
              }           
          }

          free(ql_unzip_tmp->data);
          free(ql_unzip_tmp);
          QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n",unZipFilePath);
#else 
          //???????
          QL_ZIPDEMO_LOG("========== zip file[%d] success==========\r\n",run_num+1);
          ql_rtos_task_sleep_s(5);           
          QL_ZIPDEMO_LOG("========== start unzip file[%d]==========\r\n",run_num+1);
          
          ret=ql_fs_unzip(unZipFilePath,unZip_path,1);
          if(ret!=QL_FILE_OK)
          {
                QL_ZIPDEMO_LOG("unzip file:[%s] Failed\r\n",unZipFilePath);
                goto main_exit;
          }
          
          QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n",unZipFilePath);

#endif
          QL_ZIPDEMO_LOG("========== unzip file[%d] success==========\r\n",run_num+1);

          ql_rtos_task_sleep_s(5);
    }
main_exit:
    QL_ZIPDEMO_LOG("========== zip demo end ==========\r\n");
    ql_remove(TestFilePath[0]); 
    ql_remove(TestFilePath[1]); 
    if(zip_fs_space!=NULL)
    {
       free(zip_fs_space);
       zip_fs_space = NULL;
    } 
    ql_rtos_task_delete(NULL);	
    return;
}

//application_init(ql_zip_demo_thread, "ql_zipdemo", 32, 0);

static void ql_unzip_demo_test(void *param)
{
	
	ql_rtos_task_sleep_s(5); 		  
	QL_ZIPDEMO_LOG("========== start unzip file==========\r\n");

	int ret=ql_fs_unzip(unZipFilePath,unZip_path,1);
	if(ret!=QL_FILE_OK)
	{
		 QL_ZIPDEMO_LOG("unzip file:[%s] Failed\r\n",unZipFilePath);
		 goto main_exit;
	}
	QL_ZIPDEMO_LOG("unzip file:[%s] Success\r\n",unZipFilePath);
 main_exit:
 	QL_ZIPDEMO_LOG("========== zip demo end ==========\r\n");
    ql_rtos_task_delete(NULL);	
    return;

}

	  
//application_init(ql_unzip_demo_test, "ql_zipdemo", 32, 0);


