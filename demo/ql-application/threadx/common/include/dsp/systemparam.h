#ifndef _SYSTEMPARAM__H_
#define _SYSTEMPARAM__H_

//#include "ota/ota.h"
#include "prj_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define SYSINFO_SSID_LEN_MAX		(32)
#define SYSINFO_PSK_LEN_MAX			(65)


/**
 * @brief Sysinfo station wlan parameters definition
 */
struct sysinfo_wlan_sta_param {
	uint8_t ssid[SYSINFO_SSID_LEN_MAX];
	uint8_t ssid_len;

	uint8_t psk[SYSINFO_PSK_LEN_MAX];
};


/**
 * @brief SysParam structure definition
 */
/*
 R/D��� 0:release 1��debug
*/
typedef struct sysparam {
	int battery; // ���
	int flash;   // flash
	int microphone; // ¼�� ��˷�
	int led; // led��
	int speaker; // ����
	int register_state; // �豸ע��״̬
	int volume;       // ��������	
	char firmware_version[50]; // 
	char device_secret[64]; //
	char product_key[64]; // 
	char device_name[64]; // 
	char device_SN[64]; // 
	char server_url[64]; // 
	char hwver[32]; //
	char server_mqtt[64];  // mqtt url
	int uart_log;       // log
	struct sysinfo_wlan_sta_param wlan_sta_param;
	char WifiVer[20]; // wifi
	char Topic[64];
	int backlight_level;  // 0-100
	int device_type; // 0:4G, 1:WIFI+4G
	int NetChanlLTE;
	int airkiss;
	int button_level;
	int ota;
	int first_boot;

	char server_port[8]; // mqtt server port
	char username[64];
	char password[64];
	char clientId[64];

	int  CertState;

	int ota_init;
	char larktms_url[64];
	char larktms_port[8];  // ort server port
	char larktms_mqtt_url[64];
	char larktms_mqtt_port[8];
	char larktms_secretkey[64];
	char larktms_productkey[64];
	char larktms_devname[32];
	char larktms_topic[64];

	int unuse[100];
	char Md5[16];
}sysparam_t;

extern int SYSPARAM_SIZE;//	sizeof(struct sysparam)

extern int sysparam_init(void);
extern void sysparam_deinit(void);
extern int sysparam_default(void);
extern int sysparam_save(void);
extern int sysparam_load(void);

extern struct sysparam *sysparam_get(void);
extern struct sysinfo_wlan_sta_param sysparam_get_wlanparam(void);
extern int sysparam_get_uart_log(void);
extern char * sysparam_get_device_SN(void);
extern int sysparam_get_device_type(void);
extern void sysparam_set_device_type(int value);
extern void sysparam_set_wifi_version(const char * WifiVer);

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_CHIP_SYSTEM_CHIP_H_ */


