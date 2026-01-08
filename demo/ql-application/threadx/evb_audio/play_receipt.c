#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"

#include "prj_common.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "play_receipt.h"
#include "httplayerjson.h"
#include "public_api_interface.h"

ql_task_t play_receipt_task_ctrl_thread = NULL;
ql_queue_t play_receipt_Queue = NULL;


int play_receipt_set(char* url_buf, uint8_t url_len, char* msgid_buf, uint8_t msgid_len)
{
	struct play_receipt_msg play_msg;

    memset(&play_msg, 0, sizeof(struct play_receipt_msg));
    
	play_msg.msgid_len = msgid_len;
	play_msg.url_len = url_len;
		
	play_msg.url_buf = malloc(url_len+1);
	if(play_msg.url_buf == NULL) 
		return -1;

	play_msg.msgid_buf = malloc(msgid_len+1);
	if(play_msg.msgid_buf == NULL)
	{
		free(play_msg.url_buf);
		return -1;
	}
	
	memcpy(play_msg.url_buf,url_buf,url_len);
	play_msg.url_buf[url_len]=0;
	memcpy(play_msg.msgid_buf,msgid_buf,msgid_len);
	play_msg.msgid_buf[msgid_len]=0;	
	usb_log_printf("enter %s , line %d\n", __func__, __LINE__);
	if (ql_rtos_queue_release(play_receipt_Queue, sizeof(struct play_receipt_msg), &play_msg, 0) != OS_OK)
	{
		free(play_msg.url_buf);
		free(play_msg.msgid_buf);
		usb_log_printf("%s OS_QueueSend send error\n", __func__);
		return -1;
	}
	usb_log_printf("%s OS_QueueSend send ok\n", __func__);
	return 0;
}

void xr_play_receipt_task(void *pvParameters)
{
	struct play_receipt_msg reviceMsg;
	int ResCnt = 0;
	int ret = 0;
	while(1)
	{
		if (ql_rtos_queue_wait(play_receipt_Queue, &reviceMsg, sizeof(struct play_receipt_msg), QL_WAIT_FOREVER) == OS_OK)
		{	
			usb_log_printf("%s OS_QueueSend receive ok\n", __func__);
			do
			{
				ret = DevicePlayResultSend(1, TermInfo.NetMode, reviceMsg);
				ResCnt++;
			}
			while((ret!=0)&&(ResCnt<3));
			ResCnt=0;
			if(reviceMsg.msgid_buf != NULL)
				free(reviceMsg.msgid_buf);
			if(reviceMsg.url_buf!= NULL)
				free(reviceMsg.url_buf);
		}
	}
}

void play_receipt_init(void)
{
    if (ql_rtos_queue_create(&play_receipt_Queue, sizeof(struct play_receipt_msg),50) != OS_OK) {
			usb_log_printf("ql_rtos_queue_create create play_receipt_Queue error\n");
			return ;
	}
	
    if (ql_rtos_task_create(&play_receipt_task_ctrl_thread,
							(1024*8),
							100,
							"xr_play_receipt_task",
							xr_play_receipt_task,
							NULL) != OS_OK) {
		usb_log_printf("thread create error\n");
		return ;
	}
}


