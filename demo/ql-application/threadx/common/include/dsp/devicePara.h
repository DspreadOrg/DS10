#ifndef _DEVICE_PARA_H_
#define _DEVICE_PARA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include "systemparam.h"


typedef enum {
	SETTING_OK = 0, 
	SETTING_NO_SUPPORT,		//没有设置指令
	SETTING_NO_ITEM,		//没有此设置项
	SETTING_NOT_PERMIT,		//没有操作权限
	SETTING_OVER_SIZE,		//超过设置项的大小
	SETTING_ALREADY_SET,	//无法再次写入
	SETTING_PARAM_NULL,		//参数为空
	SETTING_BUSY,			//处理忙中
} YM_SETTING_ERR_ENUM;

#define IsStr					0
#define IsIntNumber		4

//#define ITEM_ENA_R	(1<<0)
//#define ITEM_ENA_W	(1<<1)
//#define ITEM_ENA_W1	(1<<2)
//#define ITEM_ENA_RW	(ITEM_ENA_R | ITEM_ENA_W)
//#define ITEM_ENA_RW1	(ITEM_ENA_R | ITEM_ENA_W1)

typedef enum {
	ITEM_READ = 1,
	ITEM_WRITE,
	ITEM_READ_WRITE,
	ITEM_READ_WRITE1=5,
	ITEM_WRITE1=4,
} YM_SETTTING_AUTHORITY_ENUM;  // setting nvram读写权限

typedef struct{
	uint8_t authority;
	char *item_name;
	uint16_t size;
	int offset;
	uint8_t type;
}ym_setting_item_struct;

extern char const SoftSdkVer[];
extern struct sysparam *xr_sysDeviceParam;
extern const ym_setting_item_struct ym_setting_items[];

extern int readItem(char *name,uint8_t *pBuffer, uint16_t *len);
extern YM_SETTING_ERR_ENUM writeItem(char *name, uint8_t *pBuffer, uint16_t len);
extern int savePara();
extern int check_flash_para(void);

#ifdef __cplusplus
}
#endif

#endif /* _SOCKET_TASK_H_ */

