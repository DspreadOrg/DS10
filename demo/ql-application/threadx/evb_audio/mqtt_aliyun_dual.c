#include <stdio.h>
#include <math.h>
#include "ql_rtos.h"
#include "ql_power.h"
#include "ql_fs.h"
#include "MQTTClient.h"
#include "ql_spi.h"
#include "public_api_interface.h"
#include "prj_common.h"
#include "cJSON.h"
#include "mqtt_aliyun.h"
#include "systemparam.h"
#include "tts_yt_task.h"
#include "led.h"
#include "drv_bat.h"
#include "mqtt_client.h"
#include "terminfodef.h"
#include "record_store.h"
#include "ota.h"
#include "module_lte.h"
#include "httplayerjson.h"
#include "play_receipt.h"
#include "lowpower_mgmt.h"
#include "disp_port.h"
#include "utf8strdef.h"

#ifdef DUAL_MQTT_SUPPORT

#define aliot_debug_on      1
#define aliot_mes_call_on   1
#define aliot_infor_on      1

#define ALIOT_LOG(flags, fmt, arg...)   \
    do {                                \
        if (flags)                      \
            usb_log_printf(fmt, ##arg);         \
    } while (0)

#define ALIOT_DEBUG(fmt, arg...)   \
    ALIOT_LOG(aliot_debug_on, "[AliotDebug] "fmt, ##arg)

#define ALIOT_PRINT(fmt, arg...)   \
    ALIOT_LOG(aliot_mes_call_on, "[ALIOT_Print] "fmt, ##arg)

#define ALIOT_INFOR(fmt, arg...)   \
    ALIOT_LOG(aliot_infor_on, "[ALIOT_Infor] "fmt, ##arg)


//message for aliyun,define product key, device name, devcie secret
#define DEV_CLIENT_ID			"00001"

//define secure mode ;  2 : tls connect mode, 3: tcp stright connect mode
#define MQTT_TCP_MODE			"3"
#define MQTT_TLS_MODE			"2"

#define SECURE_MODE_A			MQTT_TLS_MODE
#define SECURE_MODE_B			MQTT_TCP_MODE

typedef struct {
	uint32_t		size;
	char 			*version;
	char 			*url;
	ota_verify_t	verify;
	char			*verify_value;
}ota_params;

char mqtt_ctrlA = 0;
char mqtt_ctrlB = 0;

typedef struct {
	mqt_cal cal_set;
	uint8_t mqtt_con_nums;
	uint8_t mqtt_con_rsp_times;
	Client ysw1_mqtt_client;
	aliot_devc_info_t xr_devc_info;
	xr_mqtt_param_t	xr_mqtt_para;
	char * ExitFlag;
} reconnectpara;

unsigned char mqtt_first_connet = 0;

#define OTA_THREAD_STACK_SIZE		(5 * 1024)
ql_task_t g_ota_thread = NULL;

extern char firmware_version[50];
extern char YSW1_messageId[50];
extern char YSW1_notifyUrl[200];

static char RootCA[2048] = {0};
static char DevCert[2048] = {0};
static char DevKey[2048] = {0};

void cert_load(char *rootCa,char *devCert,char *devKey)
{
	if(sysparam_get()->CertState == 0x00)
		return;
    QFILE * fp = NULL;
    int ret = -1;
	char path[64] = {0};

	do
	{
		memset(path, 0,sizeof(path));
		get_param_file_path(path, CERT_ROOTCA_FILE_NAME);
		fp = ql_fopen(path, "r");
        if (fp == NULL) {
            ALIOT_INFOR("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
			break;
        }
		 ret = ql_fread((void *)rootCa, 2048, 1, fp);
		 if (ret <= 0) {
            ALIOT_INFOR("%s_%d === read %s file failed!\n", __func__, __LINE__, path);
			break;
        }
		ql_fclose(fp);
		fp = NULL;
		memset(path, 0,sizeof(path));
		get_param_file_path(path, CERT_DEVCERT_FILE_NAME);
		fp = ql_fopen(path, "r");
        if (fp == NULL) {
            ALIOT_INFOR("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
        }
		 ret = ql_fread((void *)devCert, 2048, 1, fp);
		 if (ret <= 0) {
            ALIOT_INFOR("%s_%d === read %s file failed!\n", __func__, __LINE__, path);
			break;
        }
		memset(path, 0,sizeof(path));
		get_param_file_path(path, CERT_DEVKEY_FILE_NAME);
		fp = ql_fopen(path, "r");
		if (fp == NULL) {
            ALIOT_INFOR("%s_%d === open %s file failed!\n", __func__, __LINE__, path);
        }
		ret = ql_fread((void *)devKey, 2048, 1, fp);
		if (ret <= 0) {
            ALIOT_INFOR("%s_%d === read %s file failed!\n", __func__, __LINE__, path);
			break;
        }
	} while (0);
	
	if(fp)
	{
		ql_fclose(fp);
	}
}

void ota_end( int OtaOK )
{
//    if (ota_download_progress == 100)
    if ( OtaOK == 1 )
    {
        //tts_play_set(AudioShenjcg,AudioShenjcgLen,FixAudioTypeDef);
        tts_play_set_idx(AUD_ID_UPDATE_SUCESS,0,0);
        sysparam_get()->ota = 1;
        ALIOT_INFOR( "%s_%d ===\n", __func__, __LINE__ );
    }
    else
    {
        ALIOT_INFOR( "%s_%d ===ota failed\n", __func__, __LINE__ );

        //tts_play_set(AudioShengjsb,AudioShengjsbLen,FixAudioTypeDef);
        tts_play_set_idx(AUD_ID_UPDATE_FAIL,0,0);
        sysparam_get()->ota = 2;
    }
    ql_rtos_task_sleep_ms( 2000 );
    sysparam_save( );
    ql_rtos_task_sleep_ms( 800 );

    WaitAudioOver( );
//    if (OtaDesc)
//  	  ql_rtos_task_sleep_ms( 4000 );

	AudioPlayHalt();
    ql_power_reset( );
	AudioPlayContinue();
}

void ota_run(void *arg)
{
	ota_params *params = arg;
	uint8_t mqtt_exit_cnt = 0;

	ALIOT_PRINT("OTA begin, url %s\n", params->url);
	
	TermInfo.OTAMode = 1;
    TermLedShow(TERM_OTA_START);
    disp_set_ota_state(0);;
//	tts_play_set(AudioZhenzsjrj,AudioZhenzsjrjLen,FixAudioTypeDef);
	tts_play_set_idx(AUD_ID_UPDATE_START,0,0);
	ql_rtos_task_sleep_ms(800);
	WaitAudioPlayOver();

	MqttExit( );

	ALIOT_PRINT("mqtt_exit_cnt = %d\n", mqtt_exit_cnt);
    if ( Ext_Wifi_AppOta( params->url) == 0 )
        ota_end( 1 );
    else
        ota_end( 0 );

//    AudioPlayContinue( );
	
	if (params->version)
	{
		free(params->version);
	}
	if (params->url)
	{
		free(params->url);
	}
	if (params)
	{
		free(params);
	}
	
//	MqttExit();
	TermInfo.OTAMode=0;
	ALIOT_PRINT("OTA exit\n");
    g_ota_thread = NULL;
    ql_rtos_task_delete(NULL);
}

void _otaUpdateArrived(MessageData* data)
{
	cJSON *root=NULL, *item_data=NULL,
		*item_size, *item_version, *item_url, *item_signMethod, *item_sign;
	uint32_t size = 0;
	char *version_buff = 0, *url_buff = 0, *sign_buff = 0;
	ota_params *params = 0;
	
	ALIOT_PRINT("OTA Message arrived: %.*s: %.*s\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen,
				(char *)data->message->payload);

	if (g_ota_thread != NULL) {
		ALIOT_PRINT("OTA task is running\n");
		return;
	}

	if ((TermInfo.LowBat)&&(TermInfo.Charge==0))
	{
		return; // �͵���δ��磬��ֹ����
	}
	
	root = cJSON_Parse(data->message->payload);
	if (!root)
	{
		ALIOT_PRINT("OTA parse message failed\n");
		goto OTA_ERR;
	}
	item_data = cJSON_GetObjectItem(root, "data");
	if (!item_data)
	{
		ALIOT_PRINT("OTA not found data item\n");
		goto OTA_ERR;
	}
	//size
	item_size = cJSON_GetObjectItem(item_data, "size");
	if (!item_size)
	{
		ALIOT_PRINT("OTA not found size item\n");
		goto OTA_ERR;
	}
	size = item_size->valueint;
//	if (0)//size > PRJCONF_IMG_MAX_SIZE)
//	{
//		ALIOT_PRINT("OTA bin is too large, curr %d, max %d\n", size, PRJCONF_IMG_MAX_SIZE);
//		goto OTA_ERR;
//	}

	//version
	item_version = cJSON_GetObjectItem(item_data, "version");
	if (!item_version)
	{
		ALIOT_PRINT("OTA not found version item\n");
		goto OTA_ERR;
	}
	version_buff = malloc(strlen(item_version->valuestring) + 1);
	if (!version_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for version failed\n", strlen(item_version->valuestring));
		goto OTA_ERR;
	}
	strcpy(version_buff, item_version->valuestring);
	//url
	item_url = cJSON_GetObjectItem(item_data, "url");
	if (!item_url)
	{
		ALIOT_PRINT("OTA not found url item\n");
		goto OTA_ERR;
	}
	url_buff = malloc(strlen(item_url->valuestring) + 1);
	if (!url_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for url failed\n", strlen(item_url->valuestring));
		goto OTA_ERR;
	}
	strcpy(url_buff, item_url->valuestring);

	//signMethod
	item_signMethod = cJSON_GetObjectItem(item_data, "signMethod");
	if (!item_signMethod)
	{
		ALIOT_PRINT("OTA not found signMethod item\n");
		goto OTA_ERR;
	}

	//sign
	item_sign = cJSON_GetObjectItem(item_data, "sign");
	if (!item_sign)
	{
		ALIOT_PRINT("OTA not found sign item\n");
		goto OTA_ERR;
	}
	sign_buff = malloc(strlen(item_sign->valuestring) + 1);
	if (!sign_buff)
	{
		ALIOT_PRINT("OTA allocate %d bytes for sign failed\n", strlen(item_sign->valuestring));
		goto OTA_ERR;
	}
	strcpy(sign_buff, item_sign->valuestring);
	
	ALIOT_PRINT("OTA new firmware\n");
	ALIOT_PRINT("------------------------------\n");
	ALIOT_PRINT("\tsize: %d\n", size);
	ALIOT_PRINT("\tversion: %s\n", version_buff);
	ALIOT_PRINT("\turl: %s\n", url_buff);
	ALIOT_PRINT("\tsignMethod: %s\n", item_signMethod->valuestring);
	ALIOT_PRINT("\tsign: %s\n", sign_buff);
	ALIOT_PRINT("------------------------------\n");

	if (strcmp(version_buff, firmware_version) == 0)
	{
		ALIOT_PRINT("OTA version is same with current version, skip...\n");
		goto OTA_ERR;
	}

	params = malloc(sizeof(ota_params));
	memset(params, 0, sizeof(ota_params));
	params->size = size;
	params->version = version_buff;
	params->url = url_buff;
	if (strcmp("Md5", item_signMethod->valuestring) == 0)
	{
		params->verify = OTA_VERIFY_MD5;
		params->verify_value = sign_buff;
	}
	else if (strcmp("SHA256", item_signMethod->valuestring) == 0)
	{
		params->verify = OTA_VERIFY_SHA256;
		params->verify_value = sign_buff;
	}
	else
	{
		params->verify = OTA_VERIFY_NONE;
		params->verify_value = NULL;
		if (sign_buff)
		{
			free(sign_buff);
			sign_buff = NULL;
		}
	}
//	params->net_mode = OTA_PROTOCOL_HTTP;

	if (ql_rtos_task_create(&g_ota_thread,
						OTA_THREAD_STACK_SIZE,
						100,
						"ota_run",
						ota_run,
						params) != OS_OK) {
		ALIOT_PRINT("OTA task create failed\n");
		if (url_buff)
		{
			free(url_buff);
			url_buff = NULL;
		}
		goto OTA_ERR;
	}
	ALIOT_PRINT("OTA start, wait...\n");
	goto OTA_EXIT;

OTA_ERR:
	if (version_buff)
	{
		free(version_buff);
	}
	if (url_buff)
	{
		free(url_buff);
	}
	if (sign_buff)
	{
		free(sign_buff);
	}
	if (params)
	{
		free(params);
	}
OTA_EXIT:
	if (root)
	{
		cJSON_Delete(root);
	}
}

void otaUpdateArrivedA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
}

void otaUpdateArrivedB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	larktmsUpdateArrived(data);
}

void _messageArrived(MessageData* data)
{
#if 1
	ALIOT_PRINT("Message arrived on topic : %.*s: %.*s\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen,
				(char *)data->message->payload);
#else
	ALIOT_PRINT("Message arrived on topic : %.*s: %d\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen);
#endif

	if(DeviveMessageAnaly(data->message->payload,data->message->payloadlen) == 0)
	{	
		;
	}
}

void messageArrivedA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	_messageArrived(data);
}

void messageArrivedB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	_messageArrived(data);
}

#define MQTT_THREAD_STACK_SIZE           (1024 * 5)
#define MQTT_DISCONNECT_MAX_TIMES        40 //20
#define MQTT_YIELD_FAILED_MAX_TIMES      50
ql_task_t mqtt_task_ctrl_threadA = NULL;
ql_task_t mqtt_task_ctrl_threadB = NULL;
ql_task_t larktms_task_ctrl_threadC = NULL;

//uint8_t mqtt_con_nums = 0;
//uint8_t para->mqtt_con_rsp_times = 0;

/* 0: disconnet   1: connected   -1 : error*/
int check_mqtt_server(reconnectpara * para)
{
	int ret = -1;
	char out_stand = 0;

	if (para == NULL)
	{
		return -1;
	}
    
	out_stand = para->ysw1_mqtt_client.ping_outstanding;
	usb_log_printf("%s mqtt_con_rsp_times: %d,out_stand %d\n",__func__ ,para->mqtt_con_rsp_times,out_stand);
	if(out_stand == 1) {
		para->mqtt_con_rsp_times ++;
	}
	else {
		para->mqtt_con_rsp_times = 0;
	}

    
	//if ((mqtt_con_rsp_times > 20) || (mqtt_disconnect_times > 1000*MQTT_DISCONNECT_MAX_TIMES)) // test code
	if (para->mqtt_con_rsp_times > 20) // test code
	{
		usb_log_printf("%s mqtt_con_rsp_times = %d,  mqtt disconnect\n",__func__, para->mqtt_con_rsp_times);
		para->mqtt_con_rsp_times = 0;
		//c->mqtt_con_ping_cnt = 0; // test code
		para->cal_set.mqtt_con = MQTT_CACK;
		para->cal_set.mqtt_sub = MQTT_CACK;
		para->cal_set.mqtt_pub = MQTT_CACK;
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

void mqtt_reconnect(reconnectpara * para )
{
	int iresult=1;
	int ii;
		
	para->mqtt_con_nums ++;
    
	if(para->mqtt_con_nums == 1) {
		ii = para->mqtt_con_nums*10;
		while((ii--)&&(*para->ExitFlag))
			ql_rtos_task_sleep_ms(100);
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums==1[%d]\n", para->mqtt_con_nums);
	} else if(para->mqtt_con_nums < 7) {
		iresult = pow(2,para->mqtt_con_nums);
		ii=iresult*10;
		while((ii--)&&(*para->ExitFlag))
			ql_rtos_task_sleep_ms(100);
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums<2-6>[%d|%d]\n", para->mqtt_con_nums, iresult);
	} else if(para->mqtt_con_nums == 7) {
		para->mqtt_con_nums = 0;
		ALIOT_PRINT("mqtt_reconnect--mqtt_con_nums==7[%d]\n", para->mqtt_con_nums);
	}
}

#define MAX_MQTT_BUFF_SIZE		500

unsigned char sendbufA[MAX_MQTT_BUFF_SIZE];
unsigned char readbufA[MAX_MQTT_BUFF_SIZE];
unsigned char sendbufB[MAX_MQTT_BUFF_SIZE];
unsigned char readbufB[MAX_MQTT_BUFF_SIZE];

	char topic_sub_messageA[100] = {0};
//	char topic_pub_informA[100] = {0};
	char topic_sub_upgradeA[100] = {0};
	char topic_sub_messageB[100] = {0};
//	char topic_pub_informB[100] = {0};
	char topic_sub_upgradeB[100] = {0};

void default_msgArriveA(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	if(strstr(data->topicName->lenstring.data, topic_sub_upgradeA))
		otaUpdateArrivedA(data);
	else if(strstr(data->topicName->lenstring.data, topic_sub_messageA))
		messageArrivedA(data);
}

void default_msgArriveB(MessageData* data)
{
	usb_log_printf("======%s_%d======\n", __func__, __LINE__);
	if(strstr(data->topicName->lenstring.data, topic_sub_upgradeB))
		otaUpdateArrivedB(data);
	else if(strstr(data->topicName->lenstring.data, topic_sub_messageB))
		messageArrivedB(data);
}


char * PubTermInfo(void)
{
	cJSON *root = NULL;
	cJSON *params = NULL;
	char * pout;

	//update version for OTA start
	root = cJSON_CreateObject();
	if (root==NULL) return NULL;
	cJSON_AddItemToObject(root, "id", cJSON_CreateNumber(1));
	params = cJSON_CreateObject();
	if (params==NULL)
	{
		cJSON_Delete(root);
		return NULL;	
	}
//	cJSON_AddItemToObject(params, "version", cJSON_CreateString("1.1_1.1.0"));//cJSON_CreateString(firmware_version));
	cJSON_AddItemToObject(params, "version", cJSON_CreateString(firmware_version));
	cJSON_AddItemToObject(root, "params", params);
	pout = cJSON_Print(root);
	cJSON_Delete(root);

	return pout;
}

void mqtt_work_taskA(void * argv)
{
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	reconnectpara para;
	struct sysparam * psysconfig = sysparam_get();
	memset(&para,0,sizeof(para));
	para.ExitFlag=&mqtt_ctrlA;

	TermInfo.MqttIsRuning |= MqttMaskA;
	TermInfo.ServiceOnline &= ~MqttMaskA;
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);
	mqtt_client_close(&para.ysw1_mqtt_client);
	
	sprintf(topic_sub_messageA, "%s/%s", psysconfig->Topic, psysconfig->device_SN);
	ALIOT_PRINT(" topic_sub_messageA=%s\r\n",topic_sub_messageA);
	
	mqtt_ctrlA = 1;
	para.cal_set.mqtt_con = MQTT_CACK;
	para.cal_set.mqtt_sub = MQTT_CACK;
	para.cal_set.mqtt_pub = MQTT_CACK;

	// aliot_device_info_mulx(psysconfig->product_key, psysconfig->device_name, psysconfig->device_secret, DEV_CLIENT_ID, SECURE_MODE_A,&para.xr_devc_info);
	
	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= 60;
	connectData.cleansession		= 0;
	if(strlen(psysconfig->clientId) == 0)
		connectData.clientID.cstring 	= psysconfig->device_SN;
	else
		connectData.clientID.cstring 	= psysconfig->clientId;
	if(strlen(psysconfig->username) == 0)
		connectData.username.cstring = psysconfig->device_SN;
	else
		connectData.username.cstring 	= psysconfig->username;
	if(strlen(psysconfig->password) == 0)
		connectData.password.cstring = psysconfig->device_SN;
	else
		connectData.password.cstring = psysconfig->password;

	/* mqtt para */
	para.xr_mqtt_para.command_timeout_ms = 3000; //mqtt��ʱʱ�� 
	para.xr_mqtt_para.read_buf = readbufA;
	para.xr_mqtt_para.send_buf = sendbufA;
	para.xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.port	= atoi(psysconfig->server_port);
	
	strncpy(para.xr_mqtt_para.host_name,psysconfig->server_mqtt, strlen(psysconfig->server_mqtt));
	ql_rtos_task_sleep_s(3);

	usb_log_printf("func %s line %d ----------> transaction Mqtt para <------------\n", __func__, __LINE__);
	ALIOT_PRINT("para.xr_mqtt_para.host_name111=%s\r\n",para.xr_mqtt_para.host_name);
	usb_log_printf("func %s line %d ----------> transaction Mqtt para <------------\n", __func__, __LINE__);

	cert_load(RootCA,DevCert,DevKey);
	
	while(mqtt_ctrlA)
	{
		//ALIOT_PRINT("while(mqtt_ctrl)\n");
		if(para.cal_set.mqtt_con == MQTT_CACK) {
			TermInfo.ServiceOnline &= ~MqttMaskA;
			lpm_set(LPM_LOCK_MQTT,1);
			if(0 == memcmp(SECURE_MODE_A, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client_mulx(&para.ysw1_mqtt_client,&para.xr_mqtt_para);
			else if(0 == memcmp(SECURE_MODE_A, MQTT_TLS_MODE, 1))
			{
				if(psysconfig->CertState == 0x01)
				{
					ALIOT_PRINT("before ssl_mqtt_client_more_config, line %d\n", __LINE__);
					rc = ssl_mqtt_client_more_config(&para.ysw1_mqtt_client, RootCA, strlen(RootCA)+1, 
					DevCert, strlen(DevCert) + 1, DevKey, strlen(DevKey) + 1, NULL, 0,&para.xr_mqtt_para);
					ALIOT_PRINT("after ssl_mqtt_client_more_config, rc=%d\n", rc);
				}
				else
				{
					ALIOT_PRINT("before ssl_mqtt_client_mulx, line %d\n", __LINE__);
					rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
					ALIOT_PRINT("after ssl_mqtt_client_mulx, rc=%d\n", rc);
				}	
			}
			else
			{
				ALIOT_PRINT("before ssl_mqtt_client_mulx, line %d\n", __LINE__);
				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
				ALIOT_PRINT("after ssl_mqtt_client_mulx, rc=%d\n", rc);
			}
			para.ysw1_mqtt_client.defaultMessageHandler=default_msgArriveA;
	
			if (rc  != 0) {
				mqtt_reconnect(&para);
				continue;
			}

			rc = MQTTConnect(&para.ysw1_mqtt_client, &connectData);
			if (rc != 0) {
				ALIOT_PRINT("%s:MQTT client connect error! Return error code is %d\n",__func__, rc);
				mqtt_reconnect(&para);
				continue;
			}
			ALIOT_PRINT("%s:MQTT Connected\n",__func__);
			para.cal_set.mqtt_con = MQTT_DICACK;
		}

		if(para.cal_set.mqtt_sub == MQTT_CACK) {
			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_messageA, XR_MQTT_QOS1, messageArrivedA);
			
			if (rc != 0) {
				ALIOT_PRINT("%s:Return code from MQTT subscribe is %d\n",__func__, rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				mqtt_reconnect(&para);
				continue;
			}

			ALIOT_PRINT("%s:MQTT Subscrible is success\n",__func__);
			para.cal_set.mqtt_sub = MQTT_DICACK;

			disp_onoff_request( 1, DISP_HOLDON_MS );
            ql_rtos_task_sleep_ms( 50 );
            disp_set_service_connect_error( 0 );
            disp_sleep_enable( 1 );

			lpm_set(LPM_LOCK_MQTT,0);
		}

		if (mqtt_first_connet == 0)
		{
			tts_play_set_idx(AUD_ID_MQTT_CONNECT_SUCESS,0,0);
			mqtt_first_connet = 1;
		}



		rc = MQTTYield(&para.ysw1_mqtt_client, 3000);
		if (rc != 0)
		{
			ALIOT_PRINT("%s:Return code from yield is %d\n",__func__, rc);
			rc = MQTTDisconnect(&para.ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("%s:Return code from MQTT disconnect is %d\n",__func__,rc);
			else
				ALIOT_INFOR("%s:MQTT disconnect is success\n",__func__);


			para.cal_set.mqtt_con = MQTT_CACK;
			para.cal_set.mqtt_sub = MQTT_CACK;
			para.cal_set.mqtt_pub = MQTT_CACK;
			mqtt_reconnect(&para);
			continue;
		}

//        ql_rtos_task_sleep_ms(200);
		para.mqtt_con_nums = 0;
		check_mqtt_server(&para);
	}

	ALIOT_INFOR("%s:mqtt_exit line %d\n", __func__,__LINE__);
	
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_upgrade);
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_message);

	rc = MQTTDisconnect(&para.ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("%s:Return code from MQTT disconnect is %d\n",__func__, rc);
	else
		ALIOT_PRINT("%s:MQTT disconnect is success\n",__func__);

	mqtt_client_close(&para.ysw1_mqtt_client);
	ALIOT_PRINT("%s:mqtt_work_set end\n",__func__);
	lpm_set(LPM_LOCK_MQTT,1);
	disp_sleep_enable(0);
	TermInfo.MqttIsRuning &= ~MqttMaskA;
	TermInfo.ServiceOnline &= ~MqttMaskA;
	mqtt_task_ctrl_threadA = NULL;
	ql_rtos_task_delete(NULL);
}

void mqtt_work_taskB(void * argv)
{
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	reconnectpara para;

	char clientId[32] = {0};
	char name[32] = {0};
	int connect_repeat = 5;

	memset(&para,0,sizeof(para));
	para.ExitFlag=&mqtt_ctrlA;

	TermInfo.MqttIsRuning |= MqttMaskB;
	TermInfo.ServiceOnline &= ~MqttMaskB;
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);

	mqtt_client_close(&para.ysw1_mqtt_client);

	////////////////////
	int count = 3;
	int ret = 0;
	while(1)
	{
		ql_rtos_task_sleep_ms( 1000*2);
		if(count <= 0)
			break ;
		ret = larktms_init();

		if(ret == 0)
			break;
		
		count --;  

	}
	///////////////////
	
	mqtt_ctrlB = 1;
	para.cal_set.mqtt_con = MQTT_CACK;
	para.cal_set.mqtt_sub = MQTT_CACK;
	para.cal_set.mqtt_pub = MQTT_CACK;

	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= 300;
	connectData.cleansession		= 0;
	sprintf(clientId,"%s&%s",sysparam_get()->larktms_productkey,sysparam_get()->larktms_devname);
	connectData.clientID.cstring = clientId;
	sprintf(name,"%s&%s",sysparam_get()->larktms_productkey,sysparam_get()->larktms_devname);
	connectData.username.cstring = name;
	connectData.password.cstring = sysparam_get()->larktms_secretkey;

	/* mqtt para */
	para.xr_mqtt_para.command_timeout_ms = 3000; //mqtt��ʱʱ�� 
	para.xr_mqtt_para.read_buf = readbufB;
	para.xr_mqtt_para.send_buf = sendbufB;
	para.xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	para.xr_mqtt_para.port	= atoi(sysparam_get()->larktms_mqtt_port);
	strncpy(para.xr_mqtt_para.host_name, sysparam_get()->larktms_mqtt_url, strlen(sysparam_get()->larktms_mqtt_url));

	//topic
	strcpy(topic_sub_upgradeB, sysparam_get()->larktms_topic);
	
	ql_rtos_task_sleep_s(3);
#if 1
	if(sysparam_get()->ota_init)
	{
		usb_log_printf("func %s line %d ----------> LarkTms Mqtt para <------------\n", __func__, __LINE__);
		usb_log_printf("clientId : %s \n", connectData.clientID.cstring);
		usb_log_printf("username : %s \n", connectData.username.cstring);
		usb_log_printf("password : %s \n", connectData.password.cstring);
		usb_log_printf("host_name : %s \n", para.xr_mqtt_para.host_name);
		usb_log_printf("port : %d \n", para.xr_mqtt_para.port);
		usb_log_printf("topic : %s \n", topic_sub_upgradeB);
		usb_log_printf("func %s line %d ----------> LarkTms Mqtt para <------------\n", __func__, __LINE__);	
	}
#endif
	
	while(mqtt_ctrlB)
	{
		if(sysparam_get()->ota_init == 0)
		{
			usb_log_printf("func %s line %d ----------> LarmTms stop <------------\n", __func__, __LINE__);
			TermInfo.ServiceOnline |= MqttMaskB;
			ql_rtos_task_sleep_ms( 1000 *30);
			continue;	
		}

		if(connect_repeat <= 0)
		{
			usb_log_printf("func %s line %d ----------> connect_repeat  %d<------------\n", __func__, __LINE__,connect_repeat);
			sysparam_get()->ota_init = 0;
			sysparam_save();
			continue;
		}

		if(para.cal_set.mqtt_con == MQTT_CACK) {
		    TermInfo.ServiceOnline &= ~MqttMaskB;
		    lpm_set(LPM_LOCK_MQTT,1);
			if(0 == memcmp(SECURE_MODE_B, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client_mulx(&para.ysw1_mqtt_client,&para.xr_mqtt_para);
			if(0 == memcmp(SECURE_MODE_B, MQTT_TLS_MODE, 1))
			{
				ALIOT_PRINT("%s:before ssl_mqtt_client, line %d\n", __func__,__LINE__);
//				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,alink_ca_get(), strlen(alink_ca_get())+1);
				rc = ssl_mqtt_client_mulx(&para.ysw1_mqtt_client, &para.xr_mqtt_para,NULL, 0);
				ALIOT_PRINT("%s:after ssl_mqtt_client,line %d\n", __func__,__LINE__);
				ALIOT_PRINT("%s:ssl_mqtt_client : rc = %d \n",__func__, rc);
			}
			para.ysw1_mqtt_client.defaultMessageHandler=default_msgArriveB;
	
			if (rc  != 0) {
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}

			rc = MQTTConnect(&para.ysw1_mqtt_client, &connectData);
			if (rc != 0) {
				ALIOT_PRINT("%s:MQTT client connect error! Return error code is %d\n",__func__, rc);
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}
			ALIOT_PRINT("%s:MQTT Connected\n",__func__);
			para.cal_set.mqtt_con = MQTT_DICACK;
		}

		if(para.cal_set.mqtt_pub == MQTT_CACK) {
			
		}
		if(para.cal_set.mqtt_sub == MQTT_CACK) {
			//subscribe OTA notify start
			rc = MQTTSubscribe(&para.ysw1_mqtt_client, topic_sub_upgradeB, XR_MQTT_QOS1, otaUpdateArrivedB);
			if (rc != 0) 
			{
				ALIOT_PRINT("%s:Return code from MQTT subscribe(OTA) is %d\n",__func__, rc);
				para.cal_set.mqtt_con = MQTT_CACK;
				mqtt_reconnect(&para);
				connect_repeat --;
				continue;
			}

			ALIOT_PRINT("%s:MQTT Subscrible is success\n",__func__);
			connect_repeat = 5;
			para.cal_set.mqtt_sub = MQTT_DICACK;
		}

		TermInfo.ServiceOnline |= MqttMaskB;
		if ((TermInfo.ServiceOnline&(MqttMaskA|MqttMaskB))==(MqttMaskA|MqttMaskB))
		{
			ql_rtos_task_sleep_ms( 50 );
		}
		rc = MQTTYield(&para.ysw1_mqtt_client, 3000);
		if (rc != 0)
		{
			ALIOT_PRINT("%s:Return code from yield is %d\n",__func__, rc);
			rc = MQTTDisconnect(&para.ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("%s:Return code from MQTT disconnect is %d\n",__func__,rc);
			else
				ALIOT_INFOR("%s:MQTT disconnect is success\n",__func__);


			para.cal_set.mqtt_con = MQTT_CACK;
			para.cal_set.mqtt_sub = MQTT_CACK;
			para.cal_set.mqtt_pub = MQTT_CACK;
			mqtt_reconnect(&para);
			connect_repeat --;
			continue;
		}
//        ql_rtos_task_sleep_ms(200);
		para.mqtt_con_nums = 0;
		check_mqtt_server(&para);
	}

	ALIOT_INFOR("%s:mqtt_exit line %d\n", __func__,__LINE__);
	
	rc = MQTTDisconnect(&para.ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("%s:Return code from MQTT disconnect is %d\n",__func__, rc);
	else
		ALIOT_PRINT("%s:MQTT disconnect is success\n",__func__);

	mqtt_client_close(&para.ysw1_mqtt_client);
	ALIOT_PRINT("%s:mqtt_work_set end\n",__func__);
	lpm_set(LPM_LOCK_MQTT,1);
	disp_sleep_enable(0);
	TermInfo.ServiceOnline &= ~MqttMaskB;
	TermInfo.MqttIsRuning &= ~MqttMaskB;
	mqtt_task_ctrl_threadB = NULL;
	ql_rtos_task_delete(NULL);
}

int MqttExit(void)
{
	int ii;
	mqtt_ctrlA=0;
	mqtt_ctrlB=0;
	ALIOT_PRINT("<-- MqttExit -->\n");
	for(ii=0;ii<100;ii++)
	{
		if (!TermInfo.MqttIsRuning ) return 0;
		ql_rtos_task_sleep_ms(100);
	}
	return -1;
}

void start_Mqtt_task(void)
{
	if (!(TermInfo.MqttIsRuning&MqttMaskA))
	{
		if (ql_rtos_task_create(&mqtt_task_ctrl_threadA,
						5*1024,
						98,
						"mqtt_work_taskA",
						mqtt_work_taskA,
						NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
		
	}
	
	if (!(TermInfo.MqttIsRuning&MqttMaskB))
	{
		if (ql_rtos_task_create(&mqtt_task_ctrl_threadB,
							5*1024,
							98,
							"mqtt_work_taskB",
							mqtt_work_taskB,
							NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
	}

	if (!(TermInfo.MqttIsRuning&LarkTmsMaskC))
	{
		if (ql_rtos_task_create(&larktms_task_ctrl_threadC,
							5*1024,
							98,
							"larktms_heartbeat_work_taskC",
							larktms_heartbeat_work_taskC,
							NULL) != OS_OK)
		{
			usb_log_printf("thread create error\n");
		}
	}
}
#endif


