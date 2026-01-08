#ifndef __FIP_H__
#define __FIP_H__

#include <stdint.h>
#include <sys/types.h>

#define FIP_PLAT_FLAG_CALL_IMAGE    (1 << 0)
#define FIP_PLAT_FLAG_DISABLE_LOG   (1 << 1)

struct fip_info;
typedef struct fip_info *fip_handle;

struct fip_image;
typedef struct fip_image *fip_image_handle;

int is_valid_fip(void *address);
int fip_get_plat_flag(void *address, uint16_t *flag);

fip_handle fip_open(void *address);
int fip_close(fip_handle fh);
int fip_get_image_info(fip_handle fh, unsigned int image_id, uint64_t *pstart, uint64_t *poffset, uint64_t *psize);

fip_image_handle fip_open_image(fip_handle fh, unsigned int image_id);
int fip_close_image(fip_image_handle fih);
int64_t fip_read_image(fip_image_handle fih, uint8_t *data, uint64_t size);
void fip_handle_flag_disable_log(void *address);
int fip_get_image(void *fip_start, unsigned int img_id, uint64_t *image_addr, uint64_t *image_size);

#endif /* __FIP_H__ */
