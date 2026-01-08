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

#ifndef DUAL_MQTT_SUPPORT

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

#define SECURE_MODE			MQTT_TLS_MODE
typedef struct {
	uint32_t		size;
	char 			*version;
	char 			*url;
	ota_verify_t	verify;
	char			*verify_value;
}ota_params;

mqt_cal cal_set;
Client ysw1_mqtt_client={0,};
char mqtt_ctrl = 0;
//static int ota_download_status = -100;
//static int ota_download_progress = 0;
unsigned char mqtt_first_connet = 0;
//ql_mutex_t mqtt_lock = NULL;

#define OTA_THREAD_STACK_SIZE		(5 * 1024)
ql_task_t g_ota_thread = NULL;

extern char firmware_version[50];
extern char YSW1_messageId[50];
extern char YSW1_notifyUrl[200];



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
	//set_OTA_sta(1);

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
	//set_OTA_sta(0);

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

void otaUpdateArrived(MessageData* data)
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
		return; // ?????????����???????
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


void messageArrived(MessageData* data)
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


#define MQTT_THREAD_STACK_SIZE           (1024 * 5)
#define MQTT_DISCONNECT_MAX_TIMES        40 //20
#define MQTT_YIELD_FAILED_MAX_TIMES      50
ql_task_t mqtt_task_ctrl_thread = NULL;
uint8_t mqtt_con_nums = 0;
uint8_t mqtt_con_rsp_times = 0;

/* 0: disconnet   1: connected   -1 : error*/
int check_mqtt_server(Client * c)
{
	int ret = -1;
	char out_stand = 0;

	if (c == NULL)
	{
		return -1;
	}
    
	out_stand = c->ping_outstanding;
	usb_log_printf("%s mqtt_con_rsp_times: %d,out_stand %d\n",__func__ ,mqtt_con_rsp_times,out_stand);
	if(out_stand == 1) {
		mqtt_con_rsp_times ++;
	}
	else {
		mqtt_con_rsp_times = 0;
	}

    
	//if ((mqtt_con_rsp_times > 20) || (mqtt_disconnect_times > 1000*MQTT_DISCONNECT_MAX_TIMES)) // test code
	if (mqtt_con_rsp_times > 20) // test code
	{
		usb_log_printf("%s mqtt_con_rsp_times = %d,  mqtt disconnect\n",__func__, mqtt_con_rsp_times);
		mqtt_con_rsp_times = 0;
		//c->mqtt_con_ping_cnt = 0; // test code
		cal_set.mqtt_con = MQTT_CACK;
		cal_set.mqtt_sub = MQTT_CACK;
		cal_set.mqtt_pub = MQTT_CACK;
        ret = 0;
	}
    else
    {
        ret = 1;
    }

    return ret;
}

void mqtt_reconnect(char const * flag)
{
	int iresult=1;
	int ii;
		
	mqtt_con_nums ++;
    
	if(mqtt_con_nums == 1) {
		ii = mqtt_con_nums*10;
        ALIOT_PRINT( "mqtt con_num=%d, reconnect after %dms...\n", mqtt_con_nums,ii * 100 );
		while((ii--)&&(*flag))
			ql_rtos_task_sleep_ms(100);
	} else if(mqtt_con_nums < 7) {
		iresult = pow(2,mqtt_con_nums);
		ii=iresult*10;
        ALIOT_PRINT( "mqtt con_num=%d, reconnect after %dms...\n", mqtt_con_nums,ii * 100 );
		while((ii--)&&(*flag))
			ql_rtos_task_sleep_ms(100);
	} else if(mqtt_con_nums == 7) {
		mqtt_con_nums = 0;
        ALIOT_PRINT( "mqtt con_num=%d, reconnect after %dms...\n", mqtt_con_nums,ii * 100 );
	}
	if (mqtt_con_nums == 3 || mqtt_con_nums == 5 || mqtt_con_nums == 6) {
		disp_set_service_connect_error( 1 );
		disp_sleep_enable( 0 );
		ql_rtos_task_sleep_ms(1000);
//		tts_play_set( AudioFuwulianjieshibai, AudioFuwulianjieshibaiLen, FixAudioTypeDef );
		//tts_play_set_idx(AUD_ID_MQTT_CONNECT_FAIL,0,0);
		ALIOT_PRINT("mqtt_reconnect send tts play, line %d\n", __LINE__);
	}
}

int mqtt_msg_pub(char *topic,char *payload,int datalen,int qos)
{
    int ret;
//    if( !TermInfo.ServiceOnline || (mqtt_lock == NULL) )
//    {
//        ALIOT_PRINT("mqtt is not init\n");
//        return -1;
//    }

#if 0
    char def_topic[128];
    if( topic == NULL )
    {
        snprintf(def_topic,sizeof(def_topic),"/%s/%s/user/msg/data",sysparam_get()->product_key,sysparam_get()->device_name);
        topic = def_topic;
    }
#endif

    MQTTMessage rsp_msg;
    rsp_msg.qos = qos;
    rsp_msg.retained = 0;
    rsp_msg.payload = payload;
    rsp_msg.payloadlen = datalen;
	ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );
//    ql_rtos_mutex_lock( mqtt_lock ,QL_WAIT_FOREVER);
	
	ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );
    ret = MQTTPublish( &ysw1_mqtt_client, topic, &rsp_msg );
//    ql_rtos_mutex_unlock( mqtt_lock );
	ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );
    if ( ret != 0 )
    {
        ALIOT_PRINT( "%s: to %s fail,ret=%d\n", __func__, topic, ret );
    }
    else
    {
        ALIOT_PRINT( "%s: to %s success\n", __func__, topic );
    }

    return ret;
}

#define MAX_MQTT_BUFF_SIZE		(12 * 1024)

unsigned char sendbuf[MAX_MQTT_BUFF_SIZE];
unsigned char readbuf[MAX_MQTT_BUFF_SIZE];

char topic_sub_message[100] = { 0 };
char topic_sub_inform[100] = { 0 };
char topic_sub_upgrade[100] = { 0 };

void default_msgArrive(MessageData* data)
{
	ALIOT_PRINT("Message arrived on topic : %.*s: %d\n",
				data->topicName->lenstring.len,
				data->topicName->lenstring.data,
				data->message->payloadlen);
	if(strstr(data->topicName->lenstring.data, topic_sub_upgrade))
		otaUpdateArrived(data);
	else if(strstr(data->topicName->lenstring.data, topic_sub_message))
		messageArrived(data);
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

void mqtt_work_set(void * argv)
{
    TermInfo.MqttIsRuning = 1;
    TermInfo.ServiceOnline = 0;

	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	struct sysparam * psysconfig = sysparam_get();
	
	usb_log_printf("func %s line %d\n", __func__, __LINE__);
	mqtt_client_close(&ysw1_mqtt_client);
	memset(&ysw1_mqtt_client,0,sizeof(ysw1_mqtt_client));
	
	sprintf(topic_sub_message, "%s/%s",psysconfig->Topic,psysconfig->device_SN);
	sprintf(topic_sub_upgrade, "/ota/device/upgrade/%s/%s", psysconfig->product_key, psysconfig->device_name);
	sprintf(topic_sub_inform, "/ota/device/inform/%s/%s", psysconfig->product_key, psysconfig->device_name);
	
	mqtt_ctrl = 1;
	cal_set.mqtt_con = MQTT_CACK;
	cal_set.mqtt_sub = MQTT_CACK;
	cal_set.mqtt_pub = MQTT_CACK;
    mqtt_first_connet = 0;

	alink_devc_init();
	mqtt_para_init();

	#ifdef USE_HTTPS_GET_PARA
	rc = alink_https_set_device_info(psysconfig->product_key, psysconfig->device_name, psysconfig->device_secret, DEV_CLIENT_ID);
	if(rc == -1) {
		ALIOT_DEBUG("alink set device para error!\n");
		goto mqtt_exit;
	}
	#else
	if( strlen(psysconfig->server_mqtt) )
	{
	    // ???????
	    aliot_device_info_ex(psysconfig->server_mqtt,psysconfig->product_key, psysconfig->device_name, psysconfig->device_secret, DEV_CLIENT_ID, SECURE_MODE);
	}
	else
	{
	    // ??????
	    aliot_device_info(psysconfig->product_key, psysconfig->device_name, psysconfig->device_secret, DEV_CLIENT_ID, SECURE_MODE);
	}
	#endif

	/* connect para */
	connectData.MQTTVersion 		= 4;
	connectData.keepAliveInterval 	= (5*60*5)/4; //?????????5???????????????????????
	connectData.cleansession		= 0;
	connectData.clientID.cstring 	= psysconfig->device_SN;
	connectData.username.cstring 	= psysconfig->device_SN;
   	connectData.password.cstring 	= psysconfig->device_SN;

	/* mqtt para */
	xr_mqtt_para.command_timeout_ms = 3000; //mqtt?????? 
	xr_mqtt_para.read_buf = readbuf;
	xr_mqtt_para.send_buf = sendbuf;
	xr_mqtt_para.read_buf_size = MAX_MQTT_BUFF_SIZE;
	xr_mqtt_para.send_buf_size = MAX_MQTT_BUFF_SIZE;
	xr_mqtt_para.port	= 8883;
	strncpy(xr_mqtt_para.host_name, psysconfig->server_mqtt, strlen(psysconfig->server_mqtt));


	ALIOT_PRINT("connectData.clientID.cstring=%s\r\n",connectData.clientID.cstring);
	ALIOT_PRINT("connectData.username.cstring=%s\r\n",connectData.username.cstring);
	ALIOT_PRINT("connectData.password.cstring=%s\r\n",connectData.password.cstring);
	ALIOT_PRINT("para.xr_mqtt_para.host_name111=%s\r\n",xr_mqtt_para.host_name);

	ql_rtos_task_sleep_s(3);

	while(mqtt_ctrl)
	{
		if(cal_set.mqtt_con == MQTT_CACK)
		{
		    TermInfo.ServiceOnline = 0;
		    lpm_set(LPM_LOCK_MQTT,1);
		    if ( TermInfo.NetStatBak != NET_DEVICE_STATE_CONNECTED )
		    {
                ALIOT_PRINT( "%s: network disconnect,end task...\n", __func__ );
		        mqtt_ctrl = 0;
		        break;
		    }

		    mqtt_reconnect( &mqtt_ctrl );

			if(0 == memcmp(SECURE_MODE, MQTT_TCP_MODE, 1))
				rc = tcp_mqtt_client(&ysw1_mqtt_client);
			if(0 == memcmp(SECURE_MODE, MQTT_TLS_MODE, 1))
			{
				ALIOT_PRINT("before ssl_mqtt_client, line %d\n", __LINE__);
				rc = ssl_mqtt_client(&ysw1_mqtt_client, alink_ca_get(), strlen(alink_ca_get())+1);
				ALIOT_PRINT("after ssl_mqtt_client, line %d\n", __LINE__);
			}
			ysw1_mqtt_client.defaultMessageHandler=default_msgArrive;

			if ( rc == 0 )
            {
                rc = MQTTConnect( &ysw1_mqtt_client, &connectData );
                if ( rc == 0 )
                {
                    ALIOT_PRINT( "%s: MQTT Connected\n",__func__ );
                    cal_set.mqtt_con = MQTT_DICACK;
                    continue;
                }
                else
                {
                    ALIOT_PRINT( "MQTT client connect error! Return error code is %d\n", rc );
                }
            }
			else
			{
			    ALIOT_PRINT( "MQTT client init error! Return error code is %d\n", rc );
			}

            continue;
		}

		if(cal_set.mqtt_sub == MQTT_CACK) {
			//subscribe OTA notify start
			ALIOT_INFOR("%s,line %d,sub topic :%s \n",__func__, __LINE__,topic_sub_upgrade);
			rc = MQTTSubscribe(&ysw1_mqtt_client, topic_sub_upgrade, XR_MQTT_QOS1, otaUpdateArrived);
			if (rc != 0) 
			{
				ALIOT_PRINT("Return code from MQTT subscribe(OTA) is %d\n", rc);
				cal_set.mqtt_con = MQTT_CACK;
				continue;
			}
			ALIOT_INFOR("%s,line %d,sub topic :%s \n",__func__, __LINE__,topic_sub_message);
			//subscribe OTA notify end
			rc = MQTTSubscribe(&ysw1_mqtt_client, topic_sub_message, XR_MQTT_QOS1, messageArrived);
			
			if (rc != 0) {
				ALIOT_PRINT("Return code from MQTT subscribe is %d\n", rc);
				cal_set.mqtt_con = MQTT_CACK;
				continue;
			}

			ALIOT_PRINT("%s: sub topic %s success\n",__func__,topic_sub_message);
			cal_set.mqtt_sub = MQTT_DICACK;
		}

        if(cal_set.mqtt_pub == MQTT_CACK) {
			//update version for OTA start
			char *buff;
			buff=PubTermInfo();
			if (buff==NULL) continue;
			ALIOT_PRINT("OTA infrom:\n---------------------\n%s\n---------------------\n", buff);
			
			MQTTMessage ota_inform;
			ota_inform.qos = XR_MQTT_QOS1;
			ota_inform.retained = 0;
			ota_inform.payload = buff;
			ota_inform.payloadlen = strlen(buff);
			
			rc = MQTTPublish(&ysw1_mqtt_client, topic_sub_inform, &ota_inform);
			if (rc != 0) {
				ALIOT_PRINT("Return code from MQTT publish is %d\n", rc);
				cal_set.mqtt_con = MQTT_CACK;
				cal_set.mqtt_sub = MQTT_CACK;
				free(buff);
				continue;
			}
			free(buff);
			ALIOT_PRINT("Inform version for OTA success\n");
			//update version for OTA end

			ALIOT_PRINT("MQTT publish is success\n");
			cal_set.mqtt_pub = MQTT_DICACK;
			TermInfo.ServiceOnline = 1;

            disp_onoff_request( 1, DISP_HOLDON_MS );
            ql_rtos_task_sleep_ms( 50 );
            disp_set_service_connect_error( 0 );
            disp_sleep_enable( 1 );

            if (mqtt_first_connet == 0)
            {
                //tts_play_set(AudioFuwljcg,AudioFuwljcgLen,FixAudioTypeDef);
                tts_play_set_idx(AUD_ID_MQTT_CONNECT_SUCESS,0,0);
                mqtt_first_connet = 1;
            }
			printf("123123123123------------------------------++++++++++++++++++++++++++_________________________\r\n");
            msg_send_status(NULL);
            lpm_set(LPM_LOCK_MQTT,0);
			printf("------------------------------++++++++++++++++++++++++++_________________________\r\n");
		}
		
		ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );
//		ql_rtos_mutex_lock( mqtt_lock, QL_WAIT_FOREVER );

		ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );

		rc = MQTTYield(&ysw1_mqtt_client, 3000);
		
//		ql_rtos_mutex_unlock( mqtt_lock );
		
		ALIOT_PRINT( "%s: %d\n", __func__, __LINE__ );
		if (rc != 0)
		{
			ALIOT_PRINT("Return code from yield is %d\n", rc);
			rc = MQTTDisconnect(&ysw1_mqtt_client);
			if (rc != 0)
				ALIOT_INFOR("Return code from MQTT disconnect is %d\n", rc);
			else
				ALIOT_INFOR("MQTT disconnect is success\n");

			cal_set.mqtt_con = MQTT_CACK;
			cal_set.mqtt_sub = MQTT_CACK;
			cal_set.mqtt_pub = MQTT_CACK;

#if 0
			// ??????????????????????
			disp_set_service_connect_error(1);
			disp_sleep_enable(0);
			ql_rtos_task_sleep_ms(50);

			//timer_offline_notify = ql_rtos_get_systicks_to_ms( ) + 60 * 1000;
            //tts_play_set( AudioFuwulianjieshibai, AudioFuwulianjieshibaiLen, FixAudioTypeDef );
#endif
			// ????��?????????????????
			ql_rtos_task_sleep_ms(3000);
			continue;
		}

		mqtt_con_nums = 0;
		if( 0 == check_mqtt_server(&ysw1_mqtt_client) )
		{
#if 0
		    // ??????????????????????
            disp_set_service_connect_error( 1 );
            disp_sleep_enable( 0 );
            ql_rtos_task_sleep_ms(50);

            //timer_offline_notify = ql_rtos_get_systicks_to_ms( ) + 60 * 1000;
            //tts_play_set( AudioFuwulianjieshibai, AudioFuwulianjieshibaiLen, FixAudioTypeDef );
#endif
		}
	}
	ALIOT_INFOR("mqtt_exit line %d\n", __LINE__);
	TermInfo.ServiceOnline = 0;
	
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_upgrade);
	//MQTTUnsubscribe(&ysw1_mqtt_client, topic_sub_message);

	rc = MQTTDisconnect(&ysw1_mqtt_client);
	if (rc != 0)
		ALIOT_PRINT("Return code from MQTT disconnect is %d\n", rc);
	else
		ALIOT_PRINT("MQTT disconnect is success\n");

	mqtt_client_close(&ysw1_mqtt_client);
	ALIOT_PRINT("mqtt_work_set end\n");
	lpm_set(LPM_LOCK_MQTT,1);
	disp_sleep_enable(0);
	TermInfo.MqttIsRuning = 0;
    mqtt_task_ctrl_thread = NULL;
	ql_rtos_task_delete(NULL);
}

int MqttExit(void)
{
	int ii;
	mqtt_ctrl=0;
	ALIOT_PRINT("%s: wait mqtt exit...\n",__func__);
	for(ii=0;ii<100;ii++)
	{
		if (!TermInfo.MqttIsRuning ) return 0;
		ql_rtos_task_sleep_ms(100);
	}
	return -1;
}

void start_Mqtt_task(void)
{
	if (TermInfo.MqttIsRuning) 
    {
        ALIOT_PRINT("%s: mqtt is running or exiting,skip\n",__func__);
        return;
    }
//    if( mqtt_lock == NULL )
//    {
//        ql_rtos_mutex_create(&mqtt_lock);
//    }
	if (ql_rtos_task_create(&mqtt_task_ctrl_thread,
						10*1024,
						98,
						"mqtt_work_set",
						mqtt_work_set,
						NULL) != OS_OK)
	{
		usb_log_printf("thread create error\n");
	}
}
#endif

