#ifndef __YSW1_TIME_H__
#define __YSW1_TIME_H__

#define AUDIO_RESOURCE_DISK				'C'
#define AUDIO_RESOURCE_ROOT_PATH		"U:/" // "C:/"

int sntp_get_net_time(char const * host);
uint32_t read_rtc_time(void);
void sntp_time_task_create(void);
void audio_resource_fs_init(void);


//flash handle 
void api_flash_init(void);
int spi_flash_erase(uint32_t addr, uint32_t size);
uint32_t spi_flash_rw(uint32_t addr,void *buf, uint32_t size, int do_write);
uint32_t spi_flash_read(uint32_t addr,void *buf, uint32_t size);
uint32_t spi_flash_write(uint32_t addr,void *buf, uint32_t size);

uint32_t ql_rtos_get_systicks_to_ms(void);
uint32_t ql_rtos_get_systicks_to_s(void);
int get_host_by_name(char *name, unsigned int *address);
void usb_log_uart_tts_init(void);
void usb_log_uart_init(void);
void usb_log_printf(const char *fmt,...);

#endif
