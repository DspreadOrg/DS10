#ifndef __MODULE_GSM_H__
#define __MODULE_GSM_H__

#define NTP_ALIYUN_DOMAIN "ntp.aliyun.com"
#define NTP_ALIYUN_PORT   123

typedef struct {
	int CID;
	int LAC;
	int MCC;
	int MNC;
}struct_location;

extern int Module_Net_State(void);
extern void LTE_CAT1_init(void);
extern int set_LTE_module_low_power(void);
extern int ModelInitResult(void);
extern int GetLteIp(uint8_t *buff,uint8_t buffsize);
extern int GetGsmSignalLevel(void);

#endif

