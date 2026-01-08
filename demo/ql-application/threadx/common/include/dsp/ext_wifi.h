#ifndef __EXT_WIFI_H__
#define __EXT_WIFI_H__
#include <string.h>
#include <stdlib.h>

#define MODULE_CMD_TIMEOUT_NORMAL  		1000
#define WLAN_SSID_MAX_LEN               32
#define  RIL_MAX_URC_PREFIX_LEN         50

typedef struct{
	char ssid[128];
	char psk[128];
	int  getTimerOut;
	int (*idleFun)(void);
}at_netcfg_t;

typedef struct{
	char const * data;
	int len;
}structSend;

typedef struct wlan_ssid {
	uint8_t ssid[WLAN_SSID_MAX_LEN];
	uint8_t ssid_len;
} wlan_ssid_t;

typedef struct wlan_sta_ap {
	wlan_ssid_t	ssid;
	uint8_t		bssid[6];
	uint8_t		channel;
//	uint16_t	beacon_int;
//	int		freq;
	int		rssi;	/* unit is 0.5db */
//	int		level;	/* signal level, unit is dbm */
//	int		wpa_flags;
//	int		wpa_cipher;
//	int		wpa_key_mgmt;
//	int		wpa2_cipher;
//	int		wpa2_key_mgmt;
} wlan_sta_ap_t;

typedef struct {
    char  keyword[RIL_MAX_URC_PREFIX_LEN];
    void  (* handler)(const char* inbuf, int inlen);
}ST_URC_HDLENTRY;

//extern char Ext_Wifi_Ver[50];
extern int NetStat;

extern int AT_back_handler(char const * line, uint32_t len, void* userData) ;
extern int Wifi_SntpTime(char const * host);
extern int Wifi_GetHostByName(char const * host,int * adress);
extern int wlan_sta_set(char const * wifi_ssid,char const * wifi_key);
extern int Wifi_Que_stat(int * stat);
extern int Wifi_Self_Updata(char const * url);
extern int Ext_wifi_reBoot(void);
extern void Ext_wifi_Open(void);
void Ext_wifi_netconnect_serial_921600(void);
extern void Ext_wifi_Off(void);
extern int WifiAtChk(void);
extern int wifi_GetApInfo(wlan_sta_ap_t * ap);
extern int GetWifiSignalLevel(void);
extern void WifiEchoConfig(int enable);
extern int Wifi_ReadMac(char *mac, int str);

// wifi cfg
extern void NetConfig_json(at_netcfg_t * set,char * rbuf,int len);
extern void WifiSetApName(const char *name);
extern int wifi_ap_cfg(char const * ssid,char const * ipaddr,char const * psk,int (*cb)(void));
extern int wifi_airkiss_cfg(int timeout_ms,char const * airkiss_key,int (*cb)(void));

// URC
int URCHandler(const char* inbuf, int inlen);


#endif
