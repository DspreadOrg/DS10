/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include "ql_application.h"
#include "ql_usb_descriptor.h"
#include "ql_boot.h"
#include "ql_uart.h"
#include "ql_rtos.h"
//#define CCID_SOFT_WARE

#define APP_DEBUG(fmt, args...)	printf(fmt, ##args)
#define USB_CDC_PID_TEST           1
#define USB_CDC_PID_VALUE          0x6001
#define USB_DESCRIPTOR_TYPE_STRING 3

UINT8   usb_connect = 0;
UINT8   quec_product[] = {0x10, 0x03, 'A', 0x00, 'n', 0x00, 'd', 0x00, 'r', 0x00, 'o', 0x00, 'i', 0x00, 'd', 0x00};
UINT16  len_product = sizeof(quec_product);
UINT8   protuctid = 2;
UINT8   *prost=quec_product;

//NOTE:The callback function must not invoke any "blocking" operating system calls.
void Ql_Usb_Detect_Handler(unsigned int states)
{   

	usb_connect = states; 

	if(states)
	{	
        printf("[OPEN]USB Connect\r\n");
	} 
	else
		printf("[OPEN]USB Disconnect\r\n");
}


static void quec_usb_test(void * argv)
{   
    INT     reg=0;
	INT		usb_connect_state = -1;	
	APP_DEBUG("<------example_usb.c------>\r\n");

	//注册USB连接与断开的回调函
	ql_usbdect_register_cb(Ql_Usb_Detect_Handler);  
	   
    while(1)
    {	
        if(usb_connect)
        { 
			usb_connect = NULL;
			if(ql_usb_get_type())
			{
				APP_DEBUG("PC usb!\r\n");
			}
			else
			{
				APP_DEBUG("Adaptor usb!\r\n");
			}
        }
		usb_connect_state = ql_usb_connect_state();
		APP_DEBUG("usb_connect_state = %d\r\n",usb_connect_state);
		ql_rtos_task_sleep_s(1); 
	}
}

//application_init(quec_usb_test, "quec_usb_test", 2, 0);
#ifdef CCID_SOFT_WARE
ql_queue_t ccid_queue ;
#define ABDATA_SIZE 64
#define PC_TO_RDR_ICCPOWERON                        0x62
#define PC_TO_RDR_ICCPOWEROFF                       0x63
#define PC_TO_RDR_GETPARAMETERS                     0x6C

#define BPROTOCOL_NUM_T0  0
#define BPROTOCOL_NUM_T1  1


//CCID----->PC
#define RDR_TO_PC_DATABLOCK                         0x80
#define RDR_TO_PC_SLOTSTATUS                        0x81
#define RDR_TO_PC_PARAMETERS                        0x82
#define RDR_TO_PC_ESCAPE                            0x83
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY         0x84

#define BM_ICC_PRESENT_ACTIVE        0x00
#define BM_ICC_PRESENT_INACTIVE      0x01
#define BM_ICC_NO_ICC_PRESENT        0x02

#define BM_COMMAND_STATUS_POS        0x06
#define BM_COMMAND_STATUS_NO_ERROR   (0x00 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_FAILED     (0x01 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_TIME_EXTN  (0x02 << BM_COMMAND_STATUS_POS)

#define   SLOTERROR_BAD_LENTGH                    0x01
#define   SLOTERROR_BAD_SLOT                      0x05
#define   SLOTERROR_BAD_POWERSELECT               0x07
#define   SLOTERROR_BAD_PROTOCOLNUM               0x07
#define   SLOTERROR_BAD_CLOCKCOMMAND              0x07
#define   SLOTERROR_BAD_ABRFU_3B                  0x07
#define   SLOTERROR_BAD_BMCHANGES                 0x07
#define   SLOTERROR_BAD_BFUNCTION_MECHANICAL      0x07
#define   SLOTERROR_BAD_ABRFU_2B                  0x08
#define   SLOTERROR_BAD_LEVELPARAMETER            0x08
#define   SLOTERROR_BAD_FIDI                      0x0A
#define   SLOTERROR_BAD_T01CONVCHECKSUM           0x0B
#define   SLOTERROR_BAD_GUARDTIME                 0x0C
#define   SLOTERROR_BAD_WAITINGINTEGER            0x0D
#define   SLOTERROR_BAD_CLOCKSTOP                 0x0E
#define   SLOTERROR_BAD_IFSC                      0x0F
#define   SLOTERROR_BAD_NAD                       0x10
#define   SLOTERROR_BAD_DWLENGTH                  0x08

#define CCID_NUM_OF_SLOTS       1
#define   SLOT_NO_ERROR         0x81
#define   SLOT_NO_ERROR2        0x00

#define CCID_RESP_HDR_SIZE                10
#define RDR_TO_PC_DATABLOCK                         0x80

/* defines for the CCID_CMD Layers */

#define LEN_RDR_TO_PC_SLOTSTATUS 10
#define CCID_MSG_HDR_SIZE                 10
#define CCID_RESP_HDR_SIZE                10
//default para value for T0/1
#define   DEFAULT_FIDI              0x11
#define   DEFAULT_T01CONVCHECKSUM   0x10
#define   DEFAULT_EXTRA_GUARDTIME   0x00
#define   DEFAULT_WAITINGINTEGER    0x41
#define   DEFAULT_CLOCKSTOP         0x00
#define   DEFAULT_IFSC              0x20
#define   DEFAULT_NAD               0x00

struct ccid_bulk_out_header
{
    unsigned char bMessageType;
    unsigned int dwLength;
    unsigned char bSlot;
    unsigned char bSeq;
    unsigned char bSpecific_0;
    unsigned char bSpecific_1;
    unsigned char bSpecific_2;
    unsigned char APDU[ABDATA_SIZE];
} __attribute__((packed));
struct ccid_bulk_in_header
{
    unsigned char bMessageType;
    unsigned int dwLength;
    unsigned char bSlot;
    unsigned char bSeq;
    unsigned char bStatus;
    unsigned char bError;
    unsigned char bSpecific;
    unsigned char abData[ABDATA_SIZE];
    unsigned char bSizeToSend;
} __attribute__((packed));
typedef struct
{
    uint8_t bmFindexDindex;
    uint8_t bmTCCKST0;
    uint8_t bGuardTimeT0;
    uint8_t bWaitingIntegerT0;
    uint8_t bClockStop;
} ProtocolT0_t;


typedef struct
{
    uint8_t bmFindexDindex;
    uint8_t bmTCCKST1;
    uint8_t bGuardTimeT1;
    uint8_t bWaitingIntegerT1;
    uint8_t bClockStop;
    uint8_t bifsc;
    uint8_t bNadVal;
} ProtocolT1_t;

typedef enum
{
    CHK_PARAM_SLOT = 1,
    CHK_PARAM_DWLENGTH = (1<<1),
    CHK_PARAM_abRFU2 = (1<<2),
    CHK_PARAM_abRFU3 = (1<<3),
} QL_CMD_PARAM_CHECK_E;
	
static unsigned int ql_ccid_check_cmd_params(unsigned int check_type,struct ccid_bulk_out_header DATA,unsigned char* bStatus)
{
	*bStatus = BM_ICC_PRESENT_ACTIVE | BM_COMMAND_STATUS_NO_ERROR ;
    if (check_type & CHK_PARAM_SLOT)
    {
         /* slot num should < CCID_NUMBER_OF_SLOTs */
        if(DATA.bSlot >= CCID_NUM_OF_SLOTS)
        {
            *bStatus  = BM_COMMAND_STATUS_FAILED|BM_ICC_NO_ICC_PRESENT;
            printf("slot error");
            return SLOTERROR_BAD_SLOT;
        }
    }

    /* some cmd has no abData field DwLength should be 0 */
    if (check_type & CHK_PARAM_DWLENGTH)
    {
        if (DATA.dwLength != 0)
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("len error");
            return SLOTERROR_BAD_LENTGH;
        }
    }

        /* some cmd's abRFU field should be 0 */
    if (check_type & CHK_PARAM_abRFU2)
    {
        /* 2B abRFU Reserved for Future Use*/
        if ((DATA.bSpecific_1 != 0) ||
            (DATA.bSpecific_2 != 0))
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("abrfu2b error");
            return SLOTERROR_BAD_ABRFU_2B;
        }
    }

    /* some cmd's abRFU field should be 0 */
    if (check_type & CHK_PARAM_abRFU3)
    {
        /* 3B abRFU Reserved for Future Use*/
        if ((DATA.bSpecific_0 != 0) ||
            (DATA.bSpecific_1 != 0) ||
            (DATA.bSpecific_2 != 0))
        {
            *bStatus =BM_COMMAND_STATUS_FAILED|BM_ICC_PRESENT_ACTIVE;
            printf("abrfu3b error");
            return SLOTERROR_BAD_ABRFU_3B;
        }
    }

    return SLOT_NO_ERROR;
}
static int ql_reader2pc_data_block(unsigned char error_code,unsigned char bStatus,struct ccid_bulk_out_header data ,void *buff,unsigned short len)
{
    unsigned short length = CCID_RESP_HDR_SIZE;
	struct ccid_bulk_in_header  bulk_in_data; 
    bulk_in_data.bMessageType = RDR_TO_PC_DATABLOCK;
    bulk_in_data.bError = error_code;
    bulk_in_data.bSpecific = 0;
    bulk_in_data.dwLength = len;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    memcpy(&bulk_in_data.abData,buff,len);
    length +=bulk_in_data.dwLength;
    return ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,length);
}
static int ql_reader2pc_slot_status(uint8_t error_code,unsigned char bStatus,struct ccid_bulk_out_header data)
{
	struct ccid_bulk_in_header  bulk_in_data; 
    bulk_in_data.bMessageType = RDR_TO_PC_SLOTSTATUS;
    bulk_in_data.dwLength = 0;
    bulk_in_data.bError = error_code;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    bulk_in_data.bSpecific = 0;/* bClockStatus = 00h Clock running
                                    01h Clock stopped in state L
                                    02h Clock stopped in state H
                                    03h Clock stopped in an unknown state
                                    All other values are RFU. */
    return ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,LEN_RDR_TO_PC_SLOTSTATUS);
}
static void reader2pc_params(uint8 errorCode,unsigned char bStatus,struct ccid_bulk_out_header data,ProtocolT1_t t1_param)
{
	struct ccid_bulk_in_header	bulk_in_data; 
    uint16_t length = CCID_RESP_HDR_SIZE;
    bulk_in_data.bMessageType = RDR_TO_PC_PARAMETERS;
    bulk_in_data.bError = errorCode;
	bulk_in_data.bStatus = bStatus;
	bulk_in_data.bSeq = data.bSeq;
    bulk_in_data.bSlot = data.bSlot;
    if(errorCode == SLOT_NO_ERROR)
    {
        bulk_in_data.dwLength = sizeof(ProtocolT1_t);
        length += sizeof(ProtocolT1_t);
    }
    else
    {
        bulk_in_data.dwLength = 0;
    }
    bulk_in_data.bSpecific = BPROTOCOL_NUM_T1;
    memcpy(bulk_in_data.abData,&t1_param,sizeof(ProtocolT1_t));
    ql_uart_write(QL_USB_CDC_PORT,&bulk_in_data,length);
}


static void quec_ccid_callback(QL_UART_PORT_NUMBER_E port, void *para)
{
	int read_len = 0;
    char a[16]={0};
	struct ccid_bulk_out_header r_data = {0};
    read_len = ql_uart_read(port, &r_data, sizeof(r_data));
    ql_rtos_queue_release(ccid_queue, sizeof(r_data),&r_data, QL_NO_WAIT);
}
static void quec_ccid_startup(void * argv)
{
	/*此函数启动阶段早，禁止加打印和复杂任务*/
	ql_cdc_switch_ccid_desc(1);
    ql_rtos_queue_create(&ccid_queue, sizeof(struct ccid_bulk_out_header), 64);
	ql_uart_open(QL_USB_CDC_PORT, QL_UART_BAUD_115200, QL_FC_NONE);
    ql_uart_register_cb(QL_USB_CDC_PORT, quec_ccid_callback);	//use callback to read uart data
}

static void quec_ccid_test(void * argv)
{
  	int ret = -1;
	int write_bytes = 0;
	int read_len = 0;
	struct ccid_bulk_out_header r_data = {0};
	unsigned char bStatus = 0 ,errorCode=0 ;
	unsigned char resp_buff[13] = {0x3b,0x88,0x80,0x01,0x86,0x88,0x54,0x69,0x61,0x6e,0x59,0x75,0x19};
	ProtocolT1_t t1_param={.bmFindexDindex = DEFAULT_FIDI,
							.bmTCCKST1 = DEFAULT_T01CONVCHECKSUM,
							.bGuardTimeT1 =  DEFAULT_EXTRA_GUARDTIME,
						    .bWaitingIntegerT1 =  DEFAULT_WAITINGINTEGER,
						    .bClockStop =  DEFAULT_CLOCKSTOP,
						    .bifsc = DEFAULT_IFSC,
						    .bNadVal = DEFAULT_NAD
							};

    
	//ql_rtos_task_sleep_ms(100);
    
	while (1)
	{  
		ql_rtos_queue_wait(ccid_queue, &r_data, sizeof(r_data), QL_WAIT_FOREVER);
		switch(r_data.bMessageType)
		{
			case PC_TO_RDR_ICCPOWERON:
			    errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				printf("power on");
			    ql_reader2pc_data_block(errorCode,bStatus,r_data,(unsigned char *)resp_buff,13);
			    break;

			case PC_TO_RDR_ICCPOWEROFF:
			    errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				bStatus = BM_COMMAND_STATUS_NO_ERROR|BM_ICC_PRESENT_ACTIVE;
				printf("power off");
			    ql_reader2pc_slot_status(errorCode,bStatus,r_data);
			    break;
			case PC_TO_RDR_GETPARAMETERS:
				errorCode = errorCode = ql_ccid_check_cmd_params(CHK_PARAM_SLOT |CHK_PARAM_DWLENGTH |CHK_PARAM_abRFU3,r_data,&bStatus);
				bStatus = BM_COMMAND_STATUS_NO_ERROR|BM_ICC_PRESENT_ACTIVE;
				reader2pc_params(errorCode,bStatus,r_data,t1_param);
				break;
		}
		
		
	}    
}
//user_boot_init(quec_ccid_startup, 0);
//application_init(quec_ccid_test, "quec_ccid_test", 4, 0);
#endif