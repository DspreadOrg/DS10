#ifndef __MD5_AES_H__
#define __MD5_AES_H__

unsigned char *
aes_cbc_encrypt(const unsigned char *data, int in_len, unsigned int *out_len,
        const unsigned char *key, const unsigned char *iv);
unsigned char *
aes_cbc_decrypt(const unsigned char *data, int in_len, unsigned int *out_len,
         const unsigned char *key, const unsigned char *iv);

unsigned char *
aes_private_encrypt(const unsigned char *data, int in_len, unsigned int *out_len);
unsigned char *
aes_private_decrypt(const unsigned char *data, int in_len, unsigned int *out_len);

void md5_encrypt(char *srcbuf, uint8_t ilen, uint8_t desbuf[16]);

#endif