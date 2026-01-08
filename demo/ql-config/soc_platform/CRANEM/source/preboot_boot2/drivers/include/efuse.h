#ifndef _EFUSE_H
#define _EFUSE_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned efuse_psram(void);
int fuse_read_embed_flash(void);
void fuse_read_lotid(void);
int fuse_trust_boot_enabled(void);

#ifdef __cplusplus
}
#endif

#endif /* _EFUSE_H */
/** @} */
