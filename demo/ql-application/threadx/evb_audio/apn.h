#ifndef __APN__
#define __APN__
typedef struct APNinfo{
	const char* plmn;
	const char* apn;
	const char* user;
	const char* password;
}StructAPNinfo;

StructAPNinfo const * GetApnInfo(const char*imsi);
#endif

