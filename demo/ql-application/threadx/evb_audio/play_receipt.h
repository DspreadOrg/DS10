


#ifndef __PLAY_RECEIPT_H__
#define __PLAY_RECEIPT_H__

struct play_receipt_msg {
	char * url_buf;
	uint8_t url_len;
	char * msgid_buf;
	uint8_t msgid_len;
};

int play_receipt_set(char* url_buf, uint8_t url_len, char* msgid_buf, uint8_t msgid_len);
void play_receipt_init(void);

#endif
