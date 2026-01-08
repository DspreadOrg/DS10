/*
 * Copyright (C) 2017 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _OTA_OTA_H_
#define _OTA_OTA_H_

#include <stdio.h>
#include "ql_rtos.h"
#include "prj_common.h"
#include "MQTTClient.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OTA_OPT_EXTRA_VERIFY_CRC32      1
#define OTA_OPT_EXTRA_VERIFY_MD5        1
#define OTA_OPT_EXTRA_VERIFY_SHA1       1
#define OTA_OPT_EXTRA_VERIFY_SHA256     1

#define OTA_OPT_PROTOCOL_FILE           1
#define OTA_OPT_PROTOCOL_HTTP           1

#define LARKTMS_PARAM_FILE_PATH             "DevParam.zip"
#define CERT_ROOTCA_FILE_NAME       "ServerCA.pem"
#define CERT_DEVCERT_FILE_NAME      "deviceCert.pem"
#define CERT_DEVKEY_FILE_NAME       "deviceKey.pem"
#define PARAM_INI_FILE_PATH                 "posparam.ini"

/**
 * @brief OTA status definition
 */
typedef enum ota_status {
	OTA_STATUS_OK		= 0,
	OTA_STATUS_ERROR	= -1,
} ota_status_t;

/**
 * @brief OTA protocol definition
 */
typedef enum ota_protocol {
#if OTA_OPT_PROTOCOL_FILE
	OTA_PROTOCOL_FILE	= 0,
#endif
#if OTA_OPT_PROTOCOL_HTTP
	OTA_PROTOCOL_HTTP	= 1,
#endif

} ota_protocol_t;

/**
 * @brief OTA image verification algorithm definition
 */
typedef enum ota_verify {
	OTA_VERIFY_NONE		= 0,
#if OTA_OPT_EXTRA_VERIFY_CRC32
	OTA_VERIFY_CRC32	= 1,
#endif
#if OTA_OPT_EXTRA_VERIFY_MD5
	OTA_VERIFY_MD5		= 2,
#endif
#if OTA_OPT_EXTRA_VERIFY_SHA1
	OTA_VERIFY_SHA1		= 3,
#endif
#if OTA_OPT_EXTRA_VERIFY_SHA256
	OTA_VERIFY_SHA256	= 4,
#endif
} ota_verify_t;


/**
 * @brief OTA image verification data structure definition
 */
#define OTA_VERIFY_MAGIC        0x0055AAFF
#define OTA_VERIFY_DATA_SIZE	32
typedef struct ota_verify_data {
	uint32_t ov_magic;             /* OTA Verify Header Magic Number */
	uint16_t ov_length;            /* OTA Verify Data Length              */
	uint16_t ov_version;           /* OTA Verify Version: 0.0              */
	uint16_t ov_type;              /* OTA Verify Type                        */
	uint16_t ov_reserve;
	uint8_t ov_data[OTA_VERIFY_DATA_SIZE];
} ota_verify_data_t;

#if 0
ota_status_t ota_init(void);
void ota_deinit(void);

ota_status_t ota_get_image(ota_protocol_t protocol, void *url,void (* processhandle)(const uint8_t *,int));
ota_status_t ota_get_verify_data(ota_verify_data_t *data);
ota_status_t ota_verify_image(ota_verify_t verify, uint32_t *value);
void ota_reboot(void);
#endif

typedef struct {
	u32				heartBeatInterval;
	int				upwardPositionEnable;
	int 			inspectInterval;
	int				inspectStatus;	 
}larktms_heartbeat_param;

typedef enum task_processing_progress{
	PROGRESS_TASK_START			=0,
	PROGRESS_GET_TASK_ID 		=1,
	PROGRESS_GET_DOWNLOAD_URL 	=2,
	PROGRESS_DOWNLOADING		=3,

}task_processing_progress_e;

typedef struct {
	int				taskStatus;
	char		    taskId[64];
	int 			taskType;
	int 			subTaskType;
	int 	        forcePush;
	char			name[64];
    char            packageName[64];
	char 			downloadUrl[512]
}larktms_task_params,*plarktms_task_params;

typedef enum task_status {
	TASK_IN_PROGRESS		= 2,
	TASK_FINISH				= 3,
	TASK_FAILED 			= 4,
} task_status_e;

typedef enum subTask_status {
	SUBTASK_DOWNLOADING		= 5,
	SUBTASK_DOWNLOAD_FINISH	= 6,
	SUBTASK_DOWNLOAD_FAIL   = 7,
	SUBTASK_UPGRADE_SUCCESS = 9,
	SUBTASK_UPGRADE_FAIL	=10
} tsubTask_status_e;

int larktms_init();
void larktmsUpdateArrived(MessageData* data);
void larktms_heartbeat_work_taskC(void * argv);
void set_cust_param_path(char *path);
void get_param_file_path(char *path,char *fileName);
#ifdef __cplusplus
}
#endif

#endif /* _OTA_OTA_H_ */
