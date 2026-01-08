#ifndef __UTF8_UNICODE_H__
#define __UTF8_UNICODE_H__

unsigned short * UTF8_To_Unicode(const char *utf8_ptr,unsigned int utf8_len, int * unicode_len);	
void Hex_To_String(uint8_t * destbuf, uint16_t * srcbuf, uint16_t len);

#endif

