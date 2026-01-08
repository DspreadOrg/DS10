#ifndef __APP_BAT_H__
#define __APP_BAT_H__

#ifdef __cplusplus
	 extern "C" {
#endif

void drv_bat_init(void);
uint8_t battery_read_percent(void);

#ifdef __cplusplus
	 }
#endif

#endif

