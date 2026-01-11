#include "ota.h"
#include <stdio.h>
#include "public_api_interface.h"
#include "HTTPClient/HTTPCUsr_api.h"
#include "ql_zip.h"
#include "terminfodef.h"
#include "prj_common.h"
#include "systemparam.h"
#include "cjson.h"
#include "mqtt_aliyun.h"
#include "disp_port.h"
#include "lowpower_mgmt.h"
#include "led.h"
#include "fixaudio.h"
#include "gpio.h"

#if CFG_DBG
#define LARKTMS_DBG(...)            do{usb_log_printf("[DBG TMS]: "); usb_log_printf(__VA_ARGS__);}while(0)
#else
 #define LARKTMS_DBG(...)    do{}while(0);
#endif
#define TMS_READ_BUF_LEN 1024
#define LARKTMS_DOWNLOAD_THREAD_STACK_SIZE		(5 * 1024)

char larktms_heartbeat_ctrlC = 0;
ql_task_t g_Larktms_download_thread = NULL;

larktms_task_params g_task_params;

bool audio_pause_play = false;

char cust_param_path[32] = {0};

void get_param_file_path(char *path,char *fileName)
{
    sprintf(path,"%s%s", cust_param_path,fileName);
}

void set_cust_param_path(char *path)
{
    memset(cust_param_path,0,sizeof(cust_param_path));
    strcpy(cust_param_path,path);
}
int larktms_add_http_header(HTTPParameters *clientParams,const char * const * HeadAdd)
{
    int ret;
    char const * HeadName;
	char const * HeadData;
    while(1)
    {
        HeadName=*HeadAdd++;
        HeadData=*HeadAdd++;
        if ((HeadName==NULL)||(HeadData==NULL)) break;
        if((ret = HTTPIntrnHeadersAdd((P_HTTP_SESSION)(clientParams->pHTTP),(char *)HeadName,strlen(HeadName),
            (char *)HeadData,strlen(HeadData)))!= HTTP_CLIENT_SUCCESS)
        {
            LARKTMS_DBG("---------%s,%d---------ret = %d\r\n",__func__,__LINE__,ret);
            break;
        }
    }
    return ret;
}

int HTTPC_REQUEST(HTTPParameters *clientParams,const char * const * HeadAdd,char *recv_buf,int bufSize,int *recvSize)
{
    int ret =-1;
    unsigned int ReadLength = 0;
	HTTP_CLIENT httpClient;
    do
    {
        if ((ret = HTTPC_open(clientParams)) != 0) {
            LARKTMS_DBG("HTTPC_open err\n");
            break;
        }
        //add http header
        if (HeadAdd!=NULL)
        {
            if((ret = larktms_add_http_header(clientParams,HeadAdd)) != 0)
            {
                ret -1;
                break;
            }
        }

        if ((ret = HTTPC_request(clientParams, NULL)) != 0) {
            LARKTMS_DBG("HTTPC_request err=%d\n\n",ret);
            break;
	    }

        if ((ret = HTTPC_get_request_info(clientParams, &httpClient)) != 0) {
		    LARKTMS_DBG("HTTPC_get_request_info err\n");
		    break;
	    }

        LARKTMS_DBG("httpstatus code:%ld\n",httpClient.HTTPStatusCode);
        if (httpClient.HTTPStatusCode != HTTP_STATUS_OK) {
            ret = -2;
            break;
        }

        do {
            if ((ret = HTTPC_read(clientParams, recv_buf, bufSize, &ReadLength)) != 0) {
                if (ret == 1000) {
                    *recvSize +=ReadLength;
                    ret = 0;
                    break;
                }
                else
                    goto exit;
            }
			recv_buf +=ReadLength;
			bufSize -=ReadLength;
			*recvSize +=ReadLength;
		} while (1);
    } while (0);
exit:
    return ret;
}

int larktms_init()
{
    char *recv_buf = NULL;
    int ret;
    LARKTMS_DBG("---------%s,%d---------\r\n",__func__,__LINE__);
    unsigned int recSize = 0;
    HTTPParameters *clientParams = NULL;

    cJSON *root = NULL;
	cJSON * jsondata;
	cJSON * item;

    ret = -1;
    do
    {
        recv_buf = malloc( 1024 );
        if ( !recv_buf )
        {
            LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
            goto exit;
        }
        memset(recv_buf,0,1024);

        clientParams = malloc( sizeof( *clientParams ) );
        if ( !clientParams )
            goto exit;
        memset( clientParams, 0, sizeof(HTTPParameters) );
        //clientParams->pHTTP =() httpHeader;
        sprintf( clientParams->Uri,"%s:%s/terminal/push/connectParam/%s" ,sysparam_get()->larktms_url,sysparam_get()->larktms_port,
                         sysparam_get()->device_SN);
        LARKTMS_DBG("%s \r\n",clientParams->Uri);

        if(memcmp(clientParams->Uri,"https",5) == 0)
        {
            HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
        }
        ret = HTTPC_get( clientParams, (CHAR*) recv_buf, (INT32) TMS_READ_BUF_LEN, (INT32*) &recSize );
        LARKTMS_DBG( "HTTPC_get ret = %d\r\n" ,ret);
        if ( ret != HTTP_CLIENT_SUCCESS && ret != 1000)
        {
            ret = -1;
            break;
        }
        LARKTMS_DBG("http recv[%d]:%s \r\n",recSize,recv_buf);
        if(recSize <= 0)
        {
            ret = -1;
            break;
        }
        root = cJSON_ParseWithLength((const char*)recv_buf, recSize);
        if(root == NULL)
        {
            ret = -1;
            break;
        }

        item = cJSON_GetObjectItem(root, "code");
        if(item == 0)
        {
            ret = -1;
            break;
        }

        if(item->valueint != 0)
        {
            sysparam_get()->ota_init = 0; // larktms init fail,This action is only executed once after the first successful startup and networking
            sysparam_save();
            ret = -1;
            break;
        }

        item = cJSON_GetObjectItem(root, "data");
        if (item == 0)
        {
            ret = -1;
            break;
        }
        jsondata = item;

        item = cJSON_GetObjectItem(jsondata, "host");
        memset(sysparam_get()->larktms_mqtt_url,0,sizeof(sysparam_get()->larktms_mqtt_url));
        strcpy(sysparam_get()->larktms_mqtt_url,item->valuestring);

        item = cJSON_GetObjectItem(jsondata, "port");
        memset(sysparam_get()->larktms_mqtt_port,0,sizeof(sysparam_get()->larktms_mqtt_port));
        strcpy(sysparam_get()->larktms_mqtt_port,item->valuestring);

        item = cJSON_GetObjectItem(jsondata, "productKey");
        if(item == 0)
        {
            LARKTMS_DBG( "*** productKey is empty ***\r\n" );
        }
        memset(sysparam_get()->larktms_productkey,0,sizeof(sysparam_get()->larktms_productkey));
        strcpy(sysparam_get()->larktms_productkey,item->valuestring);
        //LARKTMS_DBG( "*** productKey is %s ***\r\n" ,item->valuestring);

        memset(sysparam_get()->larktms_devname,0,sizeof(sysparam_get()->larktms_devname));
        item = cJSON_GetObjectItem(jsondata, "deviceName");
        strcpy(sysparam_get()->larktms_devname,item->valuestring);

        memset(sysparam_get()->larktms_secretkey,0,sizeof(sysparam_get()->larktms_secretkey));
        item = cJSON_GetObjectItem(jsondata, "deviceSecret");
        strcpy(sysparam_get()->larktms_secretkey,item->valuestring);
        //LARKTMS_DBG( "*** secretkey is %s ***\r\n" ,item->valuestring);

        memset(sysparam_get()->larktms_topic,0,sizeof(sysparam_get()->larktms_topic));
        item = cJSON_GetObjectItem(jsondata, "topic");
        strcpy(sysparam_get()->larktms_topic,item->valuestring);

        sysparam_get()->ota_init = 1; // larktms init success,This action is only executed once after the first successful startup and networking
        sysparam_save();

        ret = 0;

    }while(0);

exit:
    if(root) cJSON_Delete(root);
    if(clientParams) HTTPC_close( clientParams );
	if(recv_buf) free(recv_buf);
	if(clientParams) free(clientParams);
	LARKTMS_DBG("========== tms init end ==========\r\n");
	return ret;
}

int larktms_parse_heartbeat(larktms_task_params *params,cJSON *jsontasks)
{

        char* fmt = 0;
    int count = 0;
    int i = 0;
    int ret = 0;

    cJSON* jsonitem = 0;
    cJSON* jsonchild = 0;

    const char* taskId = 0;
    int taskType = 0;
    int subTaskType = 0;
    int forcePush = 0;
    const char* name = 0;
    const char* packageName = 0;

    count = cJSON_GetArraySize(jsontasks);
    LARKTMS_DBG("<%s,%d>:count = %d",__func__,__LINE__,count);
    if(count <= 0)
    {
        return 0;
    }

    do
    {
        jsonitem = cJSON_GetArrayItem(jsontasks, count-1);
        if (jsonitem == 0) 
        {
            continue;
        }

        jsonchild = cJSON_GetObjectItem(jsonitem, "taskId");
        if (jsonchild == 0)
        {
            continue;
        }
        memcpy(params->taskId,jsonchild->valuestring,strlen(jsonchild->valuestring));

        jsonchild = cJSON_GetObjectItem(jsonitem, "taskType");
        if (jsonchild == 0)
        {
            break;
        }
        params->taskType = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(jsonitem, "subTaskType");
        if (jsonchild == 0)
        {
            break;
        }
        params->subTaskType = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(jsonitem, "forcePush");
        if (jsonchild == 0)
        {
            break;
        }
        params->forcePush = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(jsonitem, "name");
        if (jsonchild == 0)
        {
            break;
        }
        memcpy(params->name,jsonchild->valuestring,strlen(jsonchild->valuestring));

        jsonchild = cJSON_GetObjectItem(jsonitem, "packageName");
        if (jsonchild != 0)
        {
            memcpy(params->packageName,jsonchild->valuestring,strlen(jsonchild->valuestring));
        }

        ret = 1;
    }while(0);

    return ret;
}

int larktms_hearbeat(larktms_heartbeat_param *param)
{
    LARKTMS_DBG("---------%s,%d---------\r\n",__func__,__LINE__);
        char *recv_buf = NULL;
    int ret = -1;
    unsigned int ReadLength = 0;
    unsigned int recvSize = 0;
    unsigned int bufSize = 1024;
    HTTPParameters *clientParams = NULL;
	HTTP_CLIENT httpClient;
	char sendData[32] = "Hello LarkTms!";
    cJSON *root = NULL;
	cJSON * jsondata = NULL;
	cJSON * item = NULL;
    cJSON *jsontasks = NULL;

    memset((void *)&httpClient, 0, sizeof(httpClient));
    recv_buf = malloc( 1024 );
    if ( !recv_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }
    memset(recv_buf,0,1024);

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    //clientParams->pHTTP =() httpHeader;
    sprintf( clientParams->Uri,"%s:%s/terminal/heartBeat/%s" ,sysparam_get()->larktms_url,sysparam_get()->larktms_port,
                        sysparam_get()->device_SN);
    LARKTMS_DBG("%s \r\n",clientParams->Uri);

    clientParams->HttpVerb = VerbPost;
    clientParams->pData = sendData;
    clientParams->pLength = strlen(sendData);
    const char * const LarkTmsHeadAdd[]=
    {
        //"Host",sysparam_get()->larktms_url,
        "Content-Type","application/json",
        "platformVersion","3",
        "remoteDebug","0",
        "Connection","Keep-Alive",
        "apDisplayVersion",SOFTVER,
        NULL,
    };
    do
    {
        if(memcmp(clientParams->Uri,"https",5) == 0)
        {
            HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
        }

        if((ret = HTTPC_REQUEST(clientParams,LarkTmsHeadAdd,recv_buf,bufSize,&recvSize)) != 0)
        {
            ret = -1;
            break;
        }
        LARKTMS_DBG("http recv[%d]:%s \r\n",recvSize,recv_buf);

        if((root = cJSON_ParseWithLength((const char*)recv_buf, recvSize))==NULL)
        {
            ret = -1;
            break;
        }

        if( (item = cJSON_GetObjectItem(root, "code")) == 0)
        {
            ret = -1;
            break;
        }
        if(item->valueint != 0)
        {
            ret = -1;
            break;
        }

        if((item = cJSON_GetObjectItem(root, "data") )== 0)
        {
            ret = -1;
            break;
        }
        jsondata = item;

        if((item = cJSON_GetObjectItem(jsondata, "heartBeatInterval")) == 0)
        {
            ret = -1;
            break;
        }
        param->heartBeatInterval = item->valueint;

        if((item = cJSON_GetObjectItem(jsondata, "inspectInterval")) == 0)
        {
            ret = -1;
            break;
        }
        param->inspectInterval = item->valueint;

        if((item = cJSON_GetObjectItem(jsondata, "inspectStatus")) == 0)
        {
            ret = -1;
            break;
        }
        param->inspectStatus = item->valueint;

        if((jsontasks = cJSON_GetObjectItem(jsondata, "pushTasks")) ==0)
        {
            ret = -1;
            break;
        }
  
        memset(&g_task_params,0,sizeof(larktms_task_params));
        if((ret = larktms_parse_heartbeat(&g_task_params,jsontasks)) == 0)  // There are no tasks that require updating
        {
            ret = -2;
            break;
        }

        ret = 0;     //There is a new task

    }while(0);

exit:
    if(root) cJSON_Delete(root);
    if(clientParams) HTTPC_close( clientParams );
	if(recv_buf) free(recv_buf);
	if(clientParams) free(clientParams);
	LARKTMS_DBG("========== tms heartbeat end ==========\r\n");
	return ret;
}

int larktms_version_validate(plarktms_task_params param)
{
    char *recv_buf = NULL;
    int ret = -1;
    unsigned int ReadLength = 0;
    unsigned int recvSize = 0;
    unsigned int bufSize = 1024;
    uint32_t timestamp = read_rtc_time();
    HTTPParameters *clientParams = NULL;
	HTTP_CLIENT httpClient;
	char *sendData =NULL;
    cJSON *root = NULL;
	cJSON * jsondata = NULL;
	cJSON * item = NULL;
    cJSON *jsontasks = NULL;

    memset((void *)&httpClient, 0, sizeof(httpClient));
    recv_buf = malloc( 1024 );
    if ( !recv_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }
    memset(recv_buf,0,1024);

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    //clientParams->pHTTP =() httpHeader;
    sprintf( clientParams->Uri,"%s:%s/terminal/validate/%s/%s" ,sysparam_get()->larktms_url,sysparam_get()->larktms_port,
                        sysparam_get()->device_SN,param->taskId);
    LARKTMS_DBG("%s \r\n",clientParams->Uri);
    clientParams->HttpVerb = VerbPost;

    root = cJSON_CreateObject();
    if(!root)
    {
        ret = -1;
        goto exit;
    }
    // if(param->taskType == 1)
    //     cJSON_AddItemToObject(root, "name", cJSON_CreateString(CFG_LARKTMS_AP_NAME));
    // else
    //     cJSON_AddItemToObject(root, "name", cJSON_CreateString(CFG_LARKTMS_CUST_NAME));

    cJSON_AddItemToObject(root, "version", cJSON_CreateString(SOFTVER));
    sendData=cJSON_Print(root);
    cJSON_Delete(root);
    root = NULL;
    if(!sendData)
    {
        ret = -1;
        goto exit;
    }
    clientParams->pData = sendData;
    clientParams->pLength = strlen(sendData);

    const char * const LarkTmsHeadAdd[]=
    {
        "Content-Type","application/json",
        "platformVersion","3",
        "remoteDebug","0",
        "Connection","Keep-Alive",
        NULL,
    };
    do
    {
        LARKTMS_DBG("http send[%d]:%s \r\n",clientParams->pLength,clientParams->pData);
        if(memcmp(clientParams->Uri,"https",5) == 0)
        {
            HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
        }

        if((ret = HTTPC_REQUEST(clientParams,LarkTmsHeadAdd,recv_buf,bufSize,&recvSize)) != 0)
        {
            ret = -1;
            break;
        }
        LARKTMS_DBG("http recv[%d]:%s \r\n",recvSize,recv_buf);
        if((root = cJSON_ParseWithLength((const char*)recv_buf, recvSize))==NULL)
        {
            ret = -1;
            break;
        }
        if( (item = cJSON_GetObjectItem(root, "code")) == 0)
        {
            ret = -1;
            break;
        }
        if(item->valueint == 1003000106)   //The current firmware is the same as the version on the server and does not need to be updated
        {
            ret = -2;
            break;
        }
        else if(item->valueint == 0)    //Effective tasks
        {
            ret = 0;
            break;
        }
        else                            //Abnormal task
            ret = -1;
    }while(0);

exit:
    if(root) cJSON_Delete(root);
    if(clientParams) HTTPC_close( clientParams );
	if(recv_buf) free(recv_buf);
	if(clientParams) free(clientParams);
	return ret;
}

int larktms_get_download_url(plarktms_task_params param,char *downloadUrl)
{
    char *recv_buf = NULL;
    int ret = -1;
    unsigned int ReadLength = 0;
    unsigned int recvSize = 0;
    unsigned int bufSize = 1024;
    HTTPParameters *clientParams = NULL;
	HTTP_CLIENT httpClient;
	char sendData[32] = "Hello LarkTms!";
    cJSON *root = NULL;
	cJSON * jsondata = NULL;
	cJSON * item = NULL;
    cJSON *jsonApUpgradeDTO = NULL;

    memset((void *)&httpClient, 0, sizeof(httpClient));
    recv_buf = malloc( 1024 );
    if ( !recv_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }
    memset(recv_buf,0,1024);

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    sprintf( clientParams->Uri,"%s:%s/terminal/getTaskFileDownloadUrl/%s/%s" ,sysparam_get()->larktms_url,sysparam_get()->larktms_port,
                        sysparam_get()->device_SN,param->taskId);
    LARKTMS_DBG("%s \r\n",clientParams->Uri);
    clientParams->HttpVerb = VerbGet;
    clientParams->pData = sendData;
    clientParams->pLength = strlen(sendData);
    const char * const getDownloadUrlHeadAdd[]=
    {
        //"Host",sysparam_get()->larktms_url,
        "Content-Type","application/json",
        "platformVersion","3",
        "remoteDebug","0",
        "Connection","Keep-Alive",
        "taskType","1",
        "currentName",param->name,
        "currentVersion",SOFTVER,
        NULL,
    };

    do
    {
        if(memcmp(clientParams->Uri,"https",5) == 0)
        {
            HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
        }

        if((ret = HTTPC_REQUEST(clientParams,getDownloadUrlHeadAdd,recv_buf,bufSize,&recvSize)) != 0)
        {
            ret = -1;
            break;
        }
        LARKTMS_DBG("http recv[%d]:%s \r\n",recvSize,recv_buf);

        if((root = cJSON_ParseWithLength((const char*)recv_buf, recvSize))==NULL)
        {
            ret = -1;
            break;
        }

        if( (item = cJSON_GetObjectItem(root, "code")) == 0)
        {
            ret = -1;
            break;
        }
        if(item->valueint != 0)
        {
            ret = -1;
            break;
        }

        if((item = cJSON_GetObjectItem(root, "data") )== 0)
        {
            ret = -1;
            break;
        }
        jsondata = item;

        item = cJSON_GetObjectItem(jsondata, "fullUrl");
        if (item == 0)
        {
            ret = -1;
            break;
        }
        else
        {
            if(item->valuestring != NULL)
            {
                if(memcmp(item->valuestring,"https",5) == 0)
                {
                    strcpy(downloadUrl,"http");
                    strcpy(downloadUrl+4,item->valuestring+5);
                }
                else
                {
                    strcpy(downloadUrl,item->valuestring);
                }
                ret = 0;
                break;
            }
            else
            {
                ret = -1;
                break;
            }
        }
    }while(0);

exit:
    if(root) cJSON_Delete(root);
    if(clientParams) HTTPC_close( clientParams );
	if(recv_buf) free(recv_buf);
	if(clientParams) free(clientParams);
	return ret;
}

void larktms_upload_tasklog(plarktms_task_params param,char *msg,int status,int subStatus)
{
    char *recv_buf = NULL;
    int ret = -1;
    unsigned int ReadLength = 0;
    unsigned int recvSize = 0;
    unsigned int bufSize = 1024;
    uint32_t timestamp = read_rtc_time();
    HTTPParameters *clientParams = NULL;
	HTTP_CLIENT httpClient;
	char *sendData =NULL;
    cJSON *root = NULL;
	cJSON * jsondata = NULL;
	cJSON * item = NULL;
    cJSON *jsontasks = NULL;

    memset((void *)&httpClient, 0, sizeof(httpClient));
    recv_buf = malloc( 1024 );
    if ( !recv_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }
    memset(recv_buf,0,1024);

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    //clientParams->pHTTP =() httpHeader;
    sprintf( clientParams->Uri,"%s:%s/terminal/updateTaskLog/%s/%s" ,sysparam_get()->larktms_url,sysparam_get()->larktms_port,
                        sysparam_get()->device_SN,param->taskId);
    LARKTMS_DBG("%s \r\n",clientParams->Uri);
    clientParams->HttpVerb = VerbPost;

    root = cJSON_CreateObject();
    if(!root)
    {
        ret = -1;
        goto exit;
    }
    cJSON_AddItemToObject(root, "description", cJSON_CreateString(msg));
    cJSON_AddItemToObject(root, "operateTime", cJSON_CreateNumber(timestamp));
    cJSON_AddItemToObject(root, "status", cJSON_CreateNumber(status));
    cJSON_AddItemToObject(root, "subStatus", cJSON_CreateNumber(subStatus));
    sendData=cJSON_Print(root);
    cJSON_Delete(root);
    root = NULL;
    if(!sendData)
    {
        ret = -1;
        goto exit;
    }
    clientParams->pData = sendData;
    clientParams->pLength = strlen(sendData);

    const char * const LarkTmsHeadAdd[]=
    {
        "Content-Type","application/json",
        "platformVersion","3",
        "remoteDebug","0",
        "Connection","Keep-Alive",
        NULL,
    };
    do
    {
        if(memcmp(clientParams->Uri,"https",5) == 0)
        {
            HTTPC_set_ssl_verify_mode(MBEDTLS_SSL_VERIFY_NONE);
        }

        if((ret = HTTPC_REQUEST(clientParams,LarkTmsHeadAdd,recv_buf,bufSize,&recvSize)) != 0)
        {
            ret = -1;
            break;
        }
        LARKTMS_DBG("http recv[%d]:%s \r\n",recvSize,recv_buf);
        ret = 0;
    }while(0);

exit:
    if(root) cJSON_Delete(root);
    if(clientParams) HTTPC_close( clientParams );
	if(recv_buf) free(recv_buf);
	if(clientParams) free(clientParams);
	return ret;
}
#define LARKTMS_AUDIO_FILE_PATH   "AudioPackage.zip"
int Ext_Wifi_AudioOta(char * url)
{
	LARKTMS_DBG("========== update start ==========\r\n");

    QFILE * fp = NULL;
    char *file_read_buf = NULL;
    int ret;
    int filesize = 0;
    int file_read_total_len = 0;
    int per=-1;

    uint8_t eof;
    unsigned int recSize;
    HTTPParameters *clientParams = NULL;

    char path[64] = {0};

    ret = -1;
    file_read_buf = malloc( 1024 );
    if ( !file_read_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    strcpy( clientParams->Uri, url );
    audio_pause_play = true;
    get_param_file_path(path,LARKTMS_AUDIO_FILE_PATH);
    fp = ql_fopen(path, "w+");
    if (fp == NULL) {
        LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
        goto exit;
    }


    eof = 0;
    while ( 1 )
    {
        recSize=0;
        ret = HTTPC_get( clientParams, (CHAR*) file_read_buf, 1024 , (INT32*) &recSize );
        if ( ret == HTTP_CLIENT_SUCCESS )
            eof = 0;
        else if ( ret == HTTP_CLIENT_EOS )
            eof = 1;
        else
        {
            ret = -1;
            break;
        }
        if ( recSize )
        {
            file_read_total_len += recSize;
            ret = ql_fwrite((void *)file_read_buf, recSize, 1, fp);
            if(ret < 0) 
            {
                LARKTMS_DBG( "*** fota image write fail ***\r\n" );
				ret = -2;
                goto exit;
            }
			if (filesize==0)
			{
				HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
				if (HTTPIntrnHeadersFind((P_HTTP_SESSION)clientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
				{
					//char buf[30];
					//snprintf(buf,sizeof(buf),"%.*s",(int)HTTPParam.nLength,HTTPParam.pParam);
					LARKTMS_DBG("found headin:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
					sscanf(HTTPParam.pParam,"%*[^0-9]%d",&filesize);
				}
				else
				{
					printf("content-length not found!\r\n");
					filesize=700*1024;
				}
				LARKTMS_DBG("filesize:%d\r\n",filesize);
			}

			if (per!=file_read_total_len * 100/filesize)
			{
				per=file_read_total_len * 100/filesize;
				if (per>100) per=100;
				disp_set_ota_state(per);
				ql_rtos_task_sleep_ms(20);//?????
			}
        }

        if ( eof )
        {
            LARKTMS_DBG( "%s_%d =========eof ok\n", __func__, __LINE__ );
            break;
        }
    }

    LARKTMS_DBG( "%s_%d =========%d\n", __func__, __LINE__, file_read_total_len );

    ql_fclose(fp);
    fp = NULL;
    if ( ( eof ) && ( file_read_total_len ) )
    {
        ret = ql_fs_unzip(path,cust_param_path,1);
        if(!ret)
            LARKTMS_DBG( "voice package unzip success, will restart to update ...\r\n" );

//		ql_power_reset();
    }

    if ( ret < 0 )
    {
        LARKTMS_DBG( "*** dfota pkg file read fail ***\r\n" );
        goto exit;
    }
	
	
exit:
    audio_pause_play = false;
//		fota_exam_log("%s_%d =========\n", __func__, __LINE__);
    HTTPC_close( clientParams );
    if(fp)
        ql_fclose(fp);
	if(file_read_buf) free(file_read_buf);
	if(clientParams) free(clientParams);
	LARKTMS_DBG("========== update end ==========\r\n");
	return ret;
}
int update_cert()
{
    LARKTMS_DBG(" -------- update cert start------------\n");
    QFILE * fp = NULL;
    int ret = -1;
    char path[64] = {0};

    if(sysparam_get()->CertState == 0)  //ssl Do not use certificate mode
        return 0;
    do
    {
        get_param_file_path(path,CERT_ROOTCA_FILE_NAME);
        fp = ql_fopen(path, "r");
        if (fp == NULL) {
            LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
            break;
        }
        ret = 0;
    }while(0);

    if(fp)
	{
		ql_fclose(fp);
	}

    if(ret != 0 )
        return ret;

    ret = -1;
    fp = NULL;

    do
    {
        memset(path,0,sizeof(path));
        get_param_file_path(path,CERT_DEVCERT_FILE_NAME);
        fp = ql_fopen(path, "r");
        if (fp == NULL) {
            LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
            break;
        }

        ret = 0;
    }while(0);

    if(fp)
	{
		ql_fclose(fp);
	}
    if(ret != 0)
        return ret;
    
    ret = -1;
    fp = NULL;

    do
    {
        memset(path,0,sizeof(path));
        get_param_file_path(path,CERT_DEVKEY_FILE_NAME);
        fp = ql_fopen(path, "r");
        if (fp == NULL) {
            LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
            break;
        }
        ret = 0;
    }while(0);

    if(fp)
	{
		ql_fclose(fp);
	}
    LARKTMS_DBG(" -------- update cert success------------\n");
    return ret;
}

int update_param()
{
    LARKTMS_DBG(" -------- update param start------------\n");
    QFILE * fp = NULL;
    int ret = -1;
    char *file_read_buf = NULL;
    cJSON *jsonBody = NULL;
	cJSON * item = NULL;
    char path[64] = {0};
    do
    {
        get_param_file_path(path,PARAM_INI_FILE_PATH);
        fp = ql_fopen(path, "r");
        if (fp == NULL) {
            ret = 0;  //
            LARKTMS_DBG("[FS] *** no need update param\r\n");
            break;
        }
        file_read_buf = malloc( 2048 );
        memset(file_read_buf, 0, 2048);
        ret = ql_fread((void *)file_read_buf, 2048, 1, fp);
        if(ret < 0) 
    	{
    		LARKTMS_DBG("[FS] *** file read fail: %s *** or no need update param\r\n", path);
            break;
    	}
        
        if(fp)
        {
            ql_fclose(fp);
        }
        ql_remove(path);

        jsonBody = cJSON_ParseWithLength(file_read_buf,ret);

        if(jsonBody == NULL)
        {
            LARKTMS_DBG("%s data fomart error \r\n", path);
            ret = -3;
            break;
        }

        item = cJSON_GetObjectItem(jsonBody, "MqttUrl");
        if(item != NULL)
        {
            memset(sysparam_get()->server_mqtt, 0, sizeof(sysparam_get()->server_mqtt));
            strcpy(sysparam_get()->server_mqtt, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "MqttPort");
        if(item != NULL)
        {
            memset(sysparam_get()->server_port, 0, sizeof(sysparam_get()->server_port));
            strcpy(sysparam_get()->server_port, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "username");
        if(item != NULL)
        {
            memset(sysparam_get()->username, 0, sizeof(sysparam_get()->username));
            strcpy(sysparam_get()->username, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "password");
        if(item != NULL)
        {
            memset(sysparam_get()->password, 0, sizeof(sysparam_get()->password));
            strcpy(sysparam_get()->password, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "clietId");
        if(item != NULL)
        {
            memset(sysparam_get()->clientId, 0, sizeof(sysparam_get()->clientId));
            strcpy(sysparam_get()->clientId, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "Topic");
        if(item != NULL)
        {
            memset(sysparam_get()->Topic, 0, sizeof(sysparam_get()->Topic));
            strcpy(sysparam_get()->Topic, item->valuestring);
        }

        item = cJSON_GetObjectItem(jsonBody, "LogStatus");
        if(item != NULL)
        {
            sysparam_get()->uart_log = item->valueint;
        }

        item = cJSON_GetObjectItem(jsonBody, "CertState");
        if(item != NULL)
        {
            sysparam_get()->CertState = item->valueint;
        }

        sysparam_save();    
        ret = 0;
        LARKTMS_DBG(" -------- update param success------------\n");
    }while(0);

    if(file_read_buf)
        free(file_read_buf);
    if(jsonBody)
        cJSON_Delete(jsonBody);

    return ret;
}
int Ext_Wifi_CertAndParamOta(char * url)
{
	LARKTMS_DBG("========== update start ==========\r\n");

    QFILE * fp = NULL;
    char *file_read_buf = NULL;
    int ret;
    int filesize = 0;
    int file_read_total_len = 0;
    int per=-1;

    uint8_t eof;
    unsigned int recSize;
    HTTPParameters *clientParams = NULL;

    char path[64] = {0};

    ret = -1;
    file_read_buf = malloc( 1024 );
    if ( !file_read_buf )
    {
        LARKTMS_DBG( "*** heap memory is not enough ***\r\n" );
        goto exit;
    }

    clientParams = malloc( sizeof( *clientParams ) );
    if ( !clientParams )
        goto exit;
    memset( clientParams, 0, sizeof(HTTPParameters) );
    strcpy( clientParams->Uri, url );
    audio_pause_play = true;
    get_param_file_path(path,LARKTMS_PARAM_FILE_PATH);
    fp = ql_fopen(path, "w+");
    if (fp == NULL) {
        LARKTMS_DBG("%s_%d === open %s file failed!\r\n", __func__, __LINE__, path);
        goto exit;
    }


    eof = 0;
    while ( 1 )
    {
        recSize=0;
        ret = HTTPC_get( clientParams, (CHAR*) file_read_buf, 1024 , (INT32*) &recSize );
        if ( ret == HTTP_CLIENT_SUCCESS )
            eof = 0;
        else if ( ret == HTTP_CLIENT_EOS )
            eof = 1;
        else
        {
            ret = -1;
            break;
        }
        if ( recSize )
        {
            file_read_total_len += recSize;
            ret = ql_fwrite((void *)file_read_buf, recSize, 1, fp);
            if(ret < 0) 
            {
                LARKTMS_DBG( "*** fota image write fail ***\r\n" );
				ret = -2;
                goto exit;
            }
			if (filesize==0)
			{
				HTTP_PARAM      HTTPParam;                                  // A generic pointer\length parameter for parsing
				if (HTTPIntrnHeadersFind((P_HTTP_SESSION)clientParams->pHTTP,"content-length",&HTTPParam,TRUE,0)==HTTP_CLIENT_SUCCESS)
				{
					//char buf[30];
					//snprintf(buf,sizeof(buf),"%.*s",(int)HTTPParam.nLength,HTTPParam.pParam);
					LARKTMS_DBG("found headin:%.*s\r\n",(int)HTTPParam.nLength,HTTPParam.pParam);
					sscanf(HTTPParam.pParam,"%*[^0-9]%d",&filesize);
				}
				else
				{
					printf("content-length not found!\r\n");
					filesize=700*1024;
				}
				LARKTMS_DBG("filesize:%d\r\n",filesize);
			}

			if (per!=file_read_total_len * 100/filesize)
			{
				per=file_read_total_len * 100/filesize;
				if (per>100) per=100;
				disp_set_ota_state(per);
				ql_rtos_task_sleep_ms(20);//?????
			}
        }

        if ( eof )
        {
            LARKTMS_DBG( "%s_%d =========eof ok\n", __func__, __LINE__ );
            break;
        }
    }

    LARKTMS_DBG( "%s_%d =========%d\n", __func__, __LINE__, file_read_total_len );

    ql_fclose(fp);
    fp = NULL;
    if ( ( eof ) && ( file_read_total_len ) )
    {
        ret = ql_fs_unzip(path,cust_param_path,1);
        if(!ret)
            LARKTMS_DBG( "cert package unzip success, will restart to update ...\r\n" );

//		ql_power_reset();
    }

    if ( ret < 0 )
    {
        LARKTMS_DBG( "*** cert pkg file read fail ***\r\n" );
        goto exit;
    }

    do
    { 
        ret = update_param();
        if(ret != 0)
        {
            LARKTMS_DBG("update param fail\n");
            break;
        }

        ret = update_cert();
        if(ret == 0)
        {
            sysparam_get()->CertState = 0x01;
            sysparam_save();
        }
        else
        {
            ret = -1;
            break;
        }

    }while(0);

exit:
    audio_pause_play = false;
//		fota_exam_log("%s_%d =========\n", __func__, __LINE__);
    HTTPC_close( clientParams );
    if(fp)
        ql_fclose(fp);
	if(file_read_buf) free(file_read_buf);
	if(clientParams) free(clientParams);
	LARKTMS_DBG("========== update end ==========\r\n");
	return ret;
}

void larktms_download_run(void *arg)
{
    LARKTMS_DBG("========== tms dwonload start ==========\r\n");
    int ret = -1;
    plarktms_task_params param = (plarktms_task_params)arg;
    char * downLoadUrl = NULL;
    downLoadUrl = malloc(1024);
    memset(downLoadUrl,0,1024);
    if((ret = larktms_version_validate(param)) == 0)
    {
        larktms_upload_tasklog(param,"downloading",TASK_IN_PROGRESS,SUBTASK_DOWNLOADING);
        if((ret = larktms_get_download_url(param,downLoadUrl)) ==0)
        {
            param->taskStatus = PROGRESS_GET_DOWNLOAD_URL;
            larktms_taskparam_save(param);
            LARKTMS_DBG("OTA begin, url %s\n", downLoadUrl);
        
            TermInfo.OTAMode = 1;
            TermLedShow(TERM_OTA_START);
            disp_set_ota_state(0);;
            tts_play_set_idx(AUD_ID_UPDATE_START,0,0);
            ql_rtos_task_sleep_ms(800);
            WaitAudioPlayOver();

            MqttExit( );
            if(param->taskType == 1)  
                ret = Ext_Wifi_AppOta( downLoadUrl);//update firmware
            else if(param->taskType == 2)
            {
                if(param->subTaskType == 3) 
                    ret = Ext_Wifi_CertAndParamOta(downLoadUrl);// update cert and pram if forcePush == 1 will update cert else only update param
                else                               
                    ret = Ext_Wifi_AudioOta(downLoadUrl);// update audio

            }
            else
                ret = -1;        
            if (ret == 0)
            {
                //Update successful, clear task
                larktms_upload_tasklog(param,"update success",TASK_FINISH,SUBTASK_UPGRADE_SUCCESS);
                memset(param,0,sizeof(larktms_task_params));
                param->taskStatus = PROGRESS_TASK_START; 
                larktms_taskparam_save(param);
                ota_end( 1 );
            }
            else if(ret == -1)
            {
                /*
                During the download process, there was an abnormal network communication and do not clear the task. 
                After restarting, the task will continue to be executed
                */
                larktms_upload_tasklog(param,"download fail",TASK_FAILED,SUBTASK_UPGRADE_FAIL);
                ota_end( 0 );
            }
            else if(ret == -3)
            {
                //Firmware verification failed, clear this task information, do not continue with this task
                larktms_upload_tasklog(param,"firmware verify fail",TASK_FAILED,SUBTASK_UPGRADE_FAIL);
                memset(param,0,sizeof(larktms_task_params));
                param->taskStatus = PROGRESS_TASK_START;  
                larktms_taskparam_save(param);
                ota_end( 0 ); 
            }   
            else
            {
                larktms_upload_tasklog(param,"update fail",TASK_FAILED,SUBTASK_UPGRADE_FAIL);
                ota_end( 0 );
            }
        }
        else
        {
            larktms_upload_tasklog(param,"download fail",TASK_FAILED,SUBTASK_DOWNLOAD_FAIL);
            ota_end( 0 );
        }
    }
    else if(ret == -2)
    {
        larktms_upload_tasklog(param,"It's already the latest version",TASK_FINISH,SUBTASK_UPGRADE_SUCCESS);
        memset(param,0,sizeof(larktms_task_params));
        param->taskStatus = PROGRESS_TASK_START;
        larktms_taskparam_save(param);
    }
    else
    {
        larktms_upload_tasklog(param,"Invalid task",TASK_FAILED,SUBTASK_UPGRADE_FAIL);
        memset(param,0,sizeof(larktms_task_params));
        param->taskStatus = PROGRESS_TASK_START;
        larktms_taskparam_save(param);
    }
    
    if(downLoadUrl)free(downLoadUrl);

    TermInfo.OTAMode=0;
	LARKTMS_DBG("OTA exit\n");
    g_Larktms_download_thread = NULL;
    ql_rtos_task_delete(NULL);
    LARKTMS_DBG("========== tms dwonload end ==========\r\n");
}

void larktmsUpdateArrived(MessageData* data)
{
	cJSON *root=NULL, *jsonitem=NULL,*jsonchild =NULL;
	uint32_t size = 0;
    int ret;
	char *version_buff = 0, *url_buff = 0, *sign_buff = 0;

	
	LARKTMS_DBG("OTA Message arrived: %.*s: %.*s\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen,
				(char *)data->message->payload);

	if (g_Larktms_download_thread != NULL) {
		LARKTMS_DBG("Larktms download task is running\n");
		return;
	}

	if ((TermInfo.LowBat)&&(TermInfo.Charge==0))
	{
		return; // 
	}
	do
    {
        memset(&g_task_params,0,sizeof(larktms_task_params));
        root = cJSON_Parse(data->message->payload);
        if (!root)
        {
            LARKTMS_DBG("OTA parse message failed\n");
            goto OTA_EXIT;
        }
        jsonitem = cJSON_GetObjectItem(root, "taskId");
        if (jsonitem == 0)
        {
            break;
        }
        memcpy(g_task_params.taskId,jsonitem->valuestring,strlen(jsonitem->valuestring));

        jsonchild = cJSON_GetObjectItem(root, "taskType");
        if (jsonchild == 0)
        {
            break;
        }
        g_task_params.taskType = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(root, "subTaskType");
        if (jsonchild == 0)
        {
            break;
        }
        g_task_params.subTaskType = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(root, "forcePush");
        if (jsonchild == 0)
        {
            break;
        }
        g_task_params.forcePush = jsonchild->valueint;

        jsonchild = cJSON_GetObjectItem(root, "name");
        if (jsonchild == 0)
        {
            break;
        }
        memcpy(g_task_params.name,jsonchild->valuestring,strlen(jsonchild->valuestring));

        jsonchild = cJSON_GetObjectItem(root, "packageName");
        if (jsonchild != 0)
        {
            memcpy(g_task_params.packageName,jsonchild->valuestring,strlen(jsonchild->valuestring));
        }
        g_task_params.taskStatus = PROGRESS_GET_TASK_ID;
        larktms_taskparam_save(&g_task_params);
        if (ql_rtos_task_create(&g_Larktms_download_thread,
                            LARKTMS_DOWNLOAD_THREAD_STACK_SIZE,
                            100,
                            "larktms_download_run",
                            larktms_download_run,
                            &g_task_params) != OS_OK) {
            LARKTMS_DBG("OTA task create failed\n");

            goto OTA_EXIT;
        }
        LARKTMS_DBG("OTA start, wait...\n");
    }while(0);

OTA_EXIT:
	if (root)
	{
		cJSON_Delete(root);
	}
}


void larktms_heartbeat_work_taskC(void * argv)
{
    LARKTMS_DBG("-----111111111----%s,%d-----2222222222----\r\n",__func__,__LINE__);
    int ret;
    larktms_heartbeat_param hb_param;
    memset(&hb_param,0,sizeof(larktms_heartbeat_param));
    larktms_heartbeat_ctrlC = 1;

    //strcpy(sysparam_get()->device_SN,"BP110000000000000008");

    lpm_set(LPM_LOCK_MQTT,1);
    TermInfo.MqttIsRuning |= LarkTmsMaskC;
	TermInfo.ServiceOnline &= ~LarkTmsMaskC;
    get_task_status(&g_task_params);
    while(larktms_heartbeat_ctrlC)
    {
        if(g_task_params.taskStatus == PROGRESS_TASK_START)
        {
            TermInfo.ServiceOnline &= ~LarkTmsMaskC;
            ret = larktms_hearbeat(&hb_param);
            TermInfo.ServiceOnline |= LarkTmsMaskC;
            LARKTMS_DBG("---------%s,%d---------ret = %d\r\n",__func__,__LINE__,ret);
            if(ret == -2)
            {
                disp_onoff_request( 1, DISP_HOLDON_MS );
                ql_rtos_task_sleep_ms( 1000*60*hb_param.heartBeatInterval);
            }
            else if(ret == -1)
            {
                disp_onoff_request( 1, DISP_HOLDON_MS );
                ql_rtos_task_sleep_ms( 1000*60 *3);
            }
            else
            {
                g_task_params.taskStatus = PROGRESS_GET_TASK_ID; //
                larktms_taskparam_save(&g_task_params);
                if (ql_rtos_task_create(&g_Larktms_download_thread,
                        LARKTMS_DOWNLOAD_THREAD_STACK_SIZE,
                        100,
                        "larktms_download_run",
                        larktms_download_run,
                        &g_task_params) != OS_OK) {
                    LARKTMS_DBG("OTA task create failed\n");
                    goto taskExit;
                }

                disp_onoff_request( 1, DISP_HOLDON_MS );
                ql_rtos_task_sleep_ms( 1000*60*hb_param.heartBeatInterval);;
            }
        }
        else
        {
            larktms_download_run(&g_task_params);
        }
    }

taskExit:
    TermInfo.ServiceOnline &= ~LarkTmsMaskC;
	TermInfo.MqttIsRuning &= ~LarkTmsMaskC;
	larktms_heartbeat_ctrlC = NULL;
	ql_rtos_task_delete(NULL);
}

#define LARKTMS_TASK_STATUS_FILE_PATH   "U:/larktms.ini"
void larktms_taskparam_save(plarktms_task_params param)
{
    QFILE * fp = NULL;
    int ret = -1;
	int i=0;
	int cnt = 0;
	char *read_buf = NULL; //[RECORD_PAY_PER_LENGTH] = {0};

    do
    {
        fp = ql_fopen(LARKTMS_TASK_STATUS_FILE_PATH, "wb+");
        if (fp == NULL) {
            LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, LARKTMS_TASK_STATUS_FILE_PATH);
            break;
        }

        ret = ql_fwrite((void *)param, sizeof(larktms_task_params), 1, fp);
        if(ret < 0) 
    	{
    		LARKTMS_DBG("[FS] *** file write fail: %s ***\r\n", LARKTMS_TASK_STATUS_FILE_PATH);
            break;
    	}
    }while(0);

    if(fp)
	{
		ql_fclose(fp);
	}
}

int get_task_status(plarktms_task_params param)
{
    QFILE * fp = NULL;
    int ret = -1;
    memset(param,0,sizeof(larktms_task_params));
    do
    {
        fp = ql_fopen(LARKTMS_TASK_STATUS_FILE_PATH, "rb");
        if (fp == NULL) {
            LARKTMS_DBG("%s_%d === open %s file failed!\n", __func__, __LINE__, LARKTMS_TASK_STATUS_FILE_PATH);
            break;
        }

        ret = ql_fread((void *)param, sizeof(larktms_task_params), 1, fp);
        if(ret < 0) 
    	{
    		LARKTMS_DBG("[FS] *** file read fail: %s ***\r\n", LARKTMS_TASK_STATUS_FILE_PATH);
            break;
    	}
        ret = 0;
    }while(0);

    if(fp)
	{
		ql_fclose(fp);
	}

    return ret;
}