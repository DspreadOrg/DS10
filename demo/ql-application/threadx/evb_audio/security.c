#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ql_rtos.h"
#include "tl_md5.h"
#include "ym_aes.h"
#include "systemparam.h"

//QR93106541C76964   2352352345665432

#define AES_KEY_SIZE 128
#define MAX_LEN (2*1024*1024)

////#define aes_key "QR93206541C77968" //"QR93106541C76964" //
////#define aes_iv  "5693352345665860" //"2352352345665432"	//
//#if 0
////生产平台
//#define aes_key "bwccga8sbqqguj5k"
//#define aes_iv  "6qcsyucua9zgbdna"
//#else
////测试平台
//#define aes_key "g2j6fwc0ylr7syne" //"QR93106541C76964" //
//#define aes_iv  "rjw39cgnz94ctg99" //"2352352345665432"	//
//#endif
//AES密码：g2j6fwc0ylr7syne
//AES偏移量：rjw39cgnz94ctg99 
//QR939000000001~QR939000000002
#define AES_KEY "QR93206541C77968" //"QR93106541C76964" //
#define AES_IV  "5693352345665860" //"2352352345665432"	//

//#define AES_KEY (SrvPara_get()->aes_key)
//#define AES_IV  (SrvPara_get()->aes_iv)

unsigned char *
aes_cbc_encrypt(const unsigned char *data, int in_len, unsigned int *out_len,
        const unsigned char *key, const unsigned char *iv) {
    if (in_len <= 0 || in_len >= MAX_LEN) {
        return NULL;
    }
 
    if (!data) {
        return NULL;
    }
 
    unsigned int rest_len = in_len % AES_BLOCK_SIZE;
    unsigned int padding_len = AES_BLOCK_SIZE - rest_len;
    unsigned int src_len = in_len + padding_len;
 
    unsigned char *input = (unsigned char *) calloc(1, src_len);
	unsigned int i = 0;
    memcpy(input, data, in_len);
    if (padding_len > 0) {
//        memset(input + in_len, (unsigned char) padding_len, padding_len);
        for (i = 0; i < padding_len; i++) {
            *(input + in_len + i) = (unsigned char) padding_len;
        }
    }
 
    unsigned char *buff = (unsigned char *) calloc(1, src_len);
    if (!buff) {
        free(input);
        return NULL;
    }
 
    unsigned int key_schedule[AES_BLOCK_SIZE * 4] = {0};
 
    aes_key_setup(key, key_schedule, AES_KEY_SIZE);
    aes_encrypt_cbc(input, src_len, buff, key_schedule, AES_KEY_SIZE, iv);
    *out_len = src_len;
 
    //内存释放
    free(input);
 
    return buff;
}
 
unsigned char *
aes_cbc_decrypt(const unsigned char *data, int in_len, unsigned int *out_len,
         const unsigned char *key, const unsigned char *iv) {
    if (in_len <= 0 || in_len >= MAX_LEN) {
        return NULL;
    }
    if (!data) {
        return NULL;
    }
 
    unsigned int padding_len = 0;
    unsigned int src_len = in_len + padding_len;
 
    unsigned char *input = (unsigned char *) calloc(1, src_len);
	unsigned int i = 0;
    memcpy(input, data, in_len);
    if (padding_len > 0) {
//        memset(input + in_len, (unsigned char) padding_len, padding_len);
        for (i = 0; i < padding_len; i++) {
            *(input + in_len + i) = (unsigned char) padding_len;
        }
    }
 
    unsigned char *buff = (unsigned char *) calloc(1, src_len);
    if (!buff) {
        free(input);
        return NULL;
    }
 
    unsigned int key_schedule[AES_BLOCK_SIZE * 4] = {0};
 
    aes_key_setup(key, key_schedule, AES_KEY_SIZE);
    aes_decrypt_cbc(input, src_len, buff, key_schedule, AES_KEY_SIZE, iv);
 
    unsigned char *ptr = buff;
    ptr += (src_len - 1);
    padding_len = (unsigned int) *ptr;
    if (padding_len > 0 && padding_len <= AES_BLOCK_SIZE) {
        src_len -= padding_len;
    }
 
    *out_len = src_len;
 
    //内存释放
    free(input);
 
    return buff;
}

unsigned char *
aes_private_encrypt(const unsigned char *data, int in_len, unsigned int *out_len)
{
	return aes_cbc_encrypt(data, in_len, out_len, AES_KEY, AES_IV);
}

unsigned char *
aes_private_decrypt(const unsigned char *data, int in_len, unsigned int *out_len)
{
	return aes_cbc_decrypt(data, in_len, out_len, AES_KEY, AES_IV);
}

void md5_encrypt(char *srcbuf, uint8_t ilen, uint8_t desbuf[16])
{
	printf("MD5:\n");
	MD5_CTX md5;

	if ((srcbuf == NULL) || (desbuf == NULL))
	{
		printf("%s_%d ===parameter is error\n", __func__, __LINE__);
		return ;
	}

	printf("\tsrc data: %s\n", srcbuf);
	MD5Init(&md5);
	MD5Update(&md5,srcbuf,ilen);  
    MD5Final(&md5,desbuf);
	
	printf("\n");
}

