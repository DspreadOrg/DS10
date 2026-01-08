#ifndef __GLOBLE_H__
#define __GLOBLE_H__

#define OS_OK 0

typedef enum net_device_states {
	NET_DEVICE_STATE_DISCONNECTED = 0,
	NET_DEVICE_STATE_CONNECTED = 1,
} net_device_states_t;

typedef enum {
   Mode_NULL = 0,
   WIFI_MODE = 1,
   GPRS_MODE = 2,
   WIFI_BAKE_MODE = 3,
   GPRS_BAKE_MODE = 4,
}Enum_Device_Mode;

typedef struct
{
    uint8_t tm1604 :1;
    uint8_t fg7864 :1;
    uint8_t fg00ahk :1;
    uint8_t f240320 :1;
    uint8_t tm1721 :1;
} display_if_t;

typedef struct
{
    char Debug;
    display_if_t disp;
	char buttonAP;
	char SIMState;
	char SntpOk;
	//char MSntpOk;
	char NewVerFirstRun;
	char Charge;
	char ChargeOld;
	uint8_t BatRemain;
	char LowBat;
	char ChargeFull;
	char NetMode;
	int NetStatBak;
	char MqttIsRuning;
	char ServiceOnline;
	char OTAMode;
	char PlayRecordMode;
	unsigned int RepTim;
	unsigned int FailTime;
	char PlayNetSucc;
	int (*Repeat)(void);
//	void (*NetStatBreak)(void);
}StructTermInfo;

extern StructTermInfo TermInfo;
//extern int WifiParaSetSav(char const * ssid,char const * psk);



#endif
