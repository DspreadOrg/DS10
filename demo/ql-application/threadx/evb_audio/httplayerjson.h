#ifndef __HTTPLAYERJSON_H__
#define __HTTPLAYERJSON_H__

#include "play_receipt.h"
#include "HTTPClient/HTTPCUsr_api.h"

#define HTTPLAYERJSON "[HTTPLAYERJSON]: "

// 判断当前json的字符串状态是否为需要的数据
#define GETJSONSTRINGSTATE(X) 		if((NULL!=X)&&(X->type!=cJSON_String)) return -1

#define HTTPTEXTMAXSIZE 1024

//enum http_status {
//	HTTP_STATUS_CONTINUE					= 100,
//
//	HTTP_STATUS_OK						= 200,
//	HTTP_STATUS_NO_CONTENT					= 204,
//	HTTP_STATUS_PARTIAL_CONTENT				= 206,
//
//	HTTP_STATUS_MOVED_PERMANENTLY				= 301,
//	HTTP_STATUS_FOUND					= 302,
//	HTTP_STATUS_SEE_OTHER					= 303,
//	HTTP_STATUS_NOT_MODIFIED				= 304,
//
//	HTTP_STATUS_BAD_REQUEST					= 400,
//	HTTP_STATUS_UNAUTHORIZED,
//	HTTP_STATUS_PAYMENT_REQUIRED,
//	HTTP_STATUS_FORBIDDEN,
//	HTTP_STATUS_NOT_FOUND,
//	HTTP_STATUS_METHOD_NOT_ALLOWED,
//	HTTP_STATUS_NOT_ACCEPTABLE,
//	HTTP_STATUS_PROXY_AUTH_REQUIRED,
//	HTTP_STATUS_REQUEST_TIMEOUT,
//	HTTP_STATUS_CONFLICT,
//	HTTP_STATUS_GONE,
//	HTTP_STATUS_LENGTH_REQUIRED,
//	HTTP_STATUS_PRECONDITION_FAILED,
//	HTTP_STATUS_REQ_ENTITY_TOO_LARGE,
//	HTTP_STATUS_REQ_URI_TOO_LONG,
//	HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE,
//	HTTP_STATUS_REQ_RANGE_NOT_SATISFIABLE,
//	HTTP_STATUS_EXPECTATION_FAILED,
//
//	HTTP_STATUS_INTERNAL_SERVER_ERROR			= 500,
//	HTTP_STATUS_NOT_IMPLEMENTED,
//	HTTP_STATUS_BAD_GATEWAY,
//	HTTP_STATUS_SERVICE_UNAVAILABLE,
//	HTTP_STATUS_GATEWAY_TIMEOUT,
//	HTTP_STATUS_HTTP_VERSION_NOT_SUPPORTED,
//};
//
//#define HTTP_STATUS_OBJECT_MOVED                    302 // Page redirection notification
//#define HTTP_STATUS_OBJECT_MOVED_PERMANENTLY        301 // Page redirection notification
//#define HTTP_STATUS_CONTINUE                        100 // Page continue message
//
typedef struct
{
	char * url;
	char * out;
	unsigned int outlen;
	char isTransfer;
}HTTPParam;

/*  
brief: 设备注册请求
*/
int DeviveMessageAnaly(unsigned char * inbuf,int len);
int device_sign(uint8_t net_type);

/* 
brief: 设备注册接收解析
param: 需要解析的buf
*/
int DevicePlayResultSend(char PlayflagOk, uint8_t net_type, struct play_receipt_msg recvMsg);
int Gprs_HTTPC_PostB(HTTPParam *httpinfo, const char * const * HeadAdd,CHAR *Buffer,INT32 bufSize, unsigned int *recvSize);

#endif
