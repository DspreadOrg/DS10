#ifndef __PMU_H__
#define __PMU_H__

#include <stddef.h>
#include <stdint.h>

int usb_connect_with_usb_detect(void);
int usb_connect_without_usb_detect(void);
int usb_connect(void);

#endif /* __PMU_H__ */
