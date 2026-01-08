#ifndef _LOADER_H_
#define _LOADER_H_

#include <stdint.h>
#include <stdbool.h>

int boot33_loader(void *boot33_img_address, size_t boot33_img_size);
void boot33_loader_fulmar(void);
int ptable_loader(void);
bool do_boot_cb(void);
int boot2_boot_boot33(void);
void boot_xip_address(void *address);
char *volume_read_boardid(void);

#endif
