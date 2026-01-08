#ifndef _PRJ_COMMON_H_
#define _PRJ_COMMON_H_

#define OS_OK 0

#define COMMON_OK        0
#define COMMON_ERROR     -1

typedef struct
{
	uint8_t * AudioTap;
	uint8_t audio_max_level;
}StructAudioParam;

extern StructAudioParam AudioParam;


#define SPEADKERS_VOLUME_SET_MAX           4	//
#define PROFILE_IDX 1

#define PRJCONF_SYSPARAM_SAVE_TO_FLASH       1
#define PRJCONF_SYSPARAM_BACKUP              1

#define AUDIO_RESOURCE_USE_FILE_SYSTME     1

#define ONLY_PLAY_TTS_SUPPORT              0  

/* audio resource start address */
#define PRJCONF_DOWNLOAD_ADDR            (0)

#define IMAGE_INVALID_ADDR	(0xFFFFFFFF)

//#define ALIYUN_THREE_PARAMETER_MQTT_SUPPORT
#define CUST_PARAM_FILE_PATH_DS10M	"B:/CertParam/"   //tms will write cust param to this path
#define CUST_PARAM_FILE_PATH_DS10AK	"U:/CertParam/" 

#define SOFTVER "V1.0.4"

#define CFG_DBG      1

#endif

