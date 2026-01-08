#ifndef __EXT_WIFI_SOCKET_H__
#define __EXT_WIFI_SOCKET_H__

typedef struct{
	char * sbuf;
	int bufsize;
	int hasLen;
	int rCnt;
	int idx;
}structSockRead;


typedef struct{
	char const * data;
	int len;
	int total;
	int idx;
}structSockSend;

#define AEC_DISCONNECT	17

#define WIFI_ERR_NET_POOR_FAILED                     -0x0010
#define WIFI_ERR_NET_UNKNOWN_HOST                    -0x0011
#define WIFI_ERR_NET_CONN_FAILED                     -0x0012
#define WIFI_ERR_NET_INVALID_CONTEXT                 -0x0013
#define WIFI_ERR_NET_DISCONNECT                      -0x0014
#define WIFI_ERR_NET_MEM_FAILED                      -0x0015
#define WIFI_ERR_NET_BUG_FAILED                      -0x0016
#define WIFI_ERR_NET_RECV_FAILED                     -0x0017
#define WIFI_ERR_NET_SEND_FAILED                     -0x0018

void sockets_funcset(int idx);
#endif
