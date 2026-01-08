#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_data_call.h"
#include "ql_sim.h"
#include "ql_nw.h"
#include "ql_dev.h"
#include "sockets.h"
#include "netdb.h"
#include "module_lte.h"
#include "prj_common.h"
#include "public_api_interface.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "tts_yt_task.h"
#include "apn.h"

#include "cJSON.h"

//#define LOG_ERR(fmt, arg...) printf("[[ERR LTE]"fmt, ##arg)
//#define LOG_DBG(fmt, arg...) printf("[[DBG LTE]"fmt, ##arg)
//#define LOG_INFO(fmt, arg...) printf("[INFO LTE]"fmt, ##arg)
#define MODULE_GSM_ERROR(fmt, arg...) usb_log_printf("[ERR LTE]"fmt, ##arg)
#define MODULE_GSM_DEBUG(fmt, arg...) //printf("[[DBG LTE]"fmt, ##arg)
#define MODULE_GSM_INFO(fmt, arg...) usb_log_printf("[INFO LTE]"fmt, ##arg)



char ModelInitOk = 0;
ql_task_t module_net_thread = NULL;
unsigned char g_network_register_status = 0;

#define INVALID_SOCKET -1

#define DATA_CALL_PROFILE_IDX 1
//* 	ip_version	  		[in] 	IP类型：0 为IPv4，1 为IPv6，2 为IPv4v6。
#define IP_PROTOCOL 0 

typedef enum 
{
	QL_SUCCESS,
	QL_GENERIC_FAILURE,
}QL_ERROR_CODE_E;

static int data_call_successed_flag=0;
int data_call_state=-1;

static struct in_addr module_ip4_addr = {0};


#define WAIT_CHECK_SIM_STATA_MAX_NUM          60 // 120  // 60
#define WAIT_CHECK_SIM_STATA_MAX_TIME         1000 // 500 // 1000

#define WAIT_NETWORK_REGISTER_MAX_NUM         1 // 2 // 1
#define WAIT_NETWORK_REGISTER_MAX_TIME        1000 // 500 // 1000

QL_ERROR_CODE_E check_sim_state(void)
{
	int ret=0,card_status=-1,retry_num=0;
RETRY_CHECK:
	ret=ql_sim_get_card_status(&card_status);
	MODULE_GSM_INFO("The sim state: %d\n", card_status);
	if(ret==0)
	{
		switch(card_status)
		{
			case QL_SIM_STATUS_READY:
				return QL_SUCCESS;
			default:
				if(retry_num<WAIT_CHECK_SIM_STATA_MAX_NUM)
				{
					retry_num++;
					if((card_status==QL_SIM_STATUS_NOT_INSERTED)&&(retry_num==(WAIT_CHECK_SIM_STATA_MAX_NUM/2)))
					{
						ql_dev_set_modem_fun(0,0);
						ql_rtos_task_sleep_s(3); //休眠3s
						ql_dev_set_modem_fun(1,0);
						ql_rtos_task_sleep_s(2); //休眠2s
					}
					ql_rtos_task_sleep_ms(WAIT_CHECK_SIM_STATA_MAX_TIME); //休眠
					goto RETRY_CHECK;
				}
				break;
		}
	}
	return QL_GENERIC_FAILURE;
}

QL_ERROR_CODE_E wait_network_register(int time)
{
	int ret=0,retry_num=0;
	QL_NW_REG_STATUS_INFO_T  reg_status;
RETRY_CHECK:
	ret=ql_nw_get_reg_status(&reg_status);
	MODULE_GSM_INFO("The nw state: %d\n", reg_status.data_reg.state);
	if(ret==0)
	{
		switch(reg_status.data_reg.state)
		{
			case QL_NW_REG_STATE_HOME_NETWORK:
			case QL_NW_REG_STATE_ROAMING:
				return QL_SUCCESS;
			default:
				ql_rtos_task_sleep_ms(WAIT_NETWORK_REGISTER_MAX_TIME); //休眠
				if(retry_num<(time*WAIT_NETWORK_REGISTER_MAX_NUM))
				{
					retry_num++;
					goto RETRY_CHECK;
				}
				break;
		}
	}
	return QL_GENERIC_FAILURE;
}

static void datacall_status_callback(int profile_idx, int status)
{
    int ret = -1;
	struct ql_data_call_info info = {0};
	char ip_addr_str[64] = {0};
	if(status)
	{
		ret = ql_get_data_call_info(profile_idx, IP_PROTOCOL, &info);
		MODULE_GSM_DEBUG("info.profile_idx: %d\n", info.profile_idx);
		MODULE_GSM_DEBUG("info.ip_version: %d\n", info.ip_version);

        if (ret == 0)
        {
    		if(info.ip_version)
    		{
    			MODULE_GSM_DEBUG("info.v6.state: %d\n", info.v6.state);
    			MODULE_GSM_DEBUG("info.v6.reconnect: %d\n", info.v6.reconnect);

    			inet_ntop(AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.sec_dns: %s\n", ip_addr_str);
    		}
    		else
    		{
    			MODULE_GSM_INFO("info.v4.state: %d\n", info.v4.state);
    			MODULE_GSM_INFO("info.v4.reconnect: %d\n", info.v4.reconnect);

    			inet_ntop(AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.sec_dns: %s\n", ip_addr_str);

    			module_ip4_addr = info.v4.addr.ip;
    		}
            data_call_successed_flag=1;
        }
		
	}
	else
	{
		data_call_successed_flag=0;
		MODULE_GSM_DEBUG("Data call failed! profile_idx:%d\n", info.profile_idx);
	}
}

static void ql_data_call_cb(int profile_idx, int nw_status)
{
    int ret = -1;
	struct ql_data_call_info info = {0};
	char ip_addr_str[64] = {0};
    
	MODULE_GSM_INFO("call_cb: profile%d status=%d\r\n", profile_idx, nw_status);
	data_call_state=nw_status;

	if(nw_status)
	{
		ret = ql_get_data_call_info(profile_idx, IP_PROTOCOL, &info);
		MODULE_GSM_DEBUG("info.profile_idx: %d\n", info.profile_idx);
		MODULE_GSM_DEBUG("info.ip_version: %d\n", info.ip_version);

        if (ret == 0)
        {
    		if(info.ip_version)
    		{
    			MODULE_GSM_DEBUG("info.v6.state: %d\n", info.v6.state);
    			MODULE_GSM_DEBUG("info.v6.reconnect: %d\n", info.v6.reconnect);

    			inet_ntop(AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_DEBUG("info.v6.addr.sec_dns: %s\n", ip_addr_str);
    		}
    		else
    		{
    			MODULE_GSM_INFO("info.v4.state: %d\n", info.v4.state);
    			MODULE_GSM_INFO("info.v4.reconnect: %d\n", info.v4.reconnect);

    			inet_ntop(AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.ip: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.pri_dns: %s\n", ip_addr_str);

    			inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip_addr_str, sizeof(ip_addr_str));
    			MODULE_GSM_INFO("info.v4.addr.sec_dns: %s\n", ip_addr_str);

    			module_ip4_addr = info.v4.addr.ip;
    		}
            data_call_successed_flag=1;
        }
		
	}
	else
	{
		data_call_successed_flag=0;
		MODULE_GSM_DEBUG("Data call failed! profile_idx:%d\n", info.profile_idx);
	}
}
void ql_nw_notify_cb(unsigned int ind_flag, void *ind_msg_buf, unsigned int ind_msg_len, void *contextPtr)
{
    switch(ind_flag)
    {
        case QL_NW_IND_VOICE_REG_EVENT_FLAG:
        {
             QL_NW_COMMON_REG_STATUS_INFO_T  *voice_reg_status=(QL_NW_COMMON_REG_STATUS_INFO_T  *)ind_msg_buf;
                MODULE_GSM_DEBUG("QL_NW_IND_VOICE_REG_EVENT:\n\
state:%d; lac:0x%x; cid:0x%x; rat:%d; rejectCause:%d; psc:0x%x;\n", 
                        voice_reg_status->state,
                        voice_reg_status->lac,
                        voice_reg_status->cid,
                        voice_reg_status->rat,
                        voice_reg_status->rejectCause,
                        voice_reg_status->psc);
        }
        break;
        case QL_NW_IND_DATA_REG_EVENT_FLAG:
        {
             QL_NW_COMMON_REG_STATUS_INFO_T  *data_reg_status=(QL_NW_COMMON_REG_STATUS_INFO_T  *)ind_msg_buf;
                MODULE_GSM_DEBUG("QL_NW_IND_DATA_REG_EVENT:\n\
state:%d; lac:0x%x; cid:0x%x; rat:%d; rejectCause:%d;\n", 
                        data_reg_status->state,
                        data_reg_status->lac,
                        data_reg_status->cid,
                        data_reg_status->rat,
                        data_reg_status->rejectCause);
                
				if ((data_reg_status->state == 1)||(data_reg_status->state == 5))
				{
					data_call_successed_flag = 1;
				}
                else
                {
                    data_call_successed_flag = 0;
                }
        }
        break;
        case QL_NW_IND_SIGNAL_STRENGTH_EVENT_FLAG:
        {
            QL_NW_SIGNAL_STRENGTH_INFO_T *signal=(QL_NW_SIGNAL_STRENGTH_INFO_T *)ind_msg_buf;
            MODULE_GSM_DEBUG("QL_NW_IND_SIGNAL_STRENGTH_EVENT:\n");
            MODULE_GSM_INFO("GW: rssi:%d; bitErrorRate:%d; rscp:%d; ecno:%d;\n",
            signal->GW_SignalStrength.rssi,
            signal->GW_SignalStrength.bitErrorRate,
            signal->GW_SignalStrength.rscp,
            signal->GW_SignalStrength.ecno);
            MODULE_GSM_INFO("LTE: rssi:%d; rsrp:%d; rsrq:%d; cqi:%d;\n",
            signal->LTE_SignalStrength.rssi,
            signal->LTE_SignalStrength.rsrp,
            signal->LTE_SignalStrength.rsrq,
            signal->LTE_SignalStrength.cqi);
        }
        break;
        case QL_NW_IND_NITZ_TIME_UPDATE_EVENT_FLAG:
        {
            QL_NW_NITZ_TIME_INFO_T  *time_info=(QL_NW_NITZ_TIME_INFO_T *)ind_msg_buf;
            MODULE_GSM_DEBUG("QL_NW_IND_NITZ_TIME_UPDATE_EVENT:\nnitz_time:%s; abs_time:%ld; leap_sec:%d\n",time_info->nitz_time,time_info->abs_time,time_info->leap_sec);
            MODULE_GSM_DEBUG("now time:%ld\n",time(NULL));
        }
        break;
    }
}

static 	const StructAPNinfo dApn=	{NULL, "3gnet", NULL, NULL};
QL_ERROR_CODE_E start_data_call(void)
{
	int ret = QL_SUCCESS;
	char apn_name[32] = {0};
//	char PLMN[8] = {0};
    char IMSI[32] = {0};
    int i = 0;
	StructAPNinfo const * pAPNinfo;
	
	data_call_successed_flag=0;
	
	MODULE_GSM_INFO("start_data_call:%d\n", __LINE__);
	ret=ql_wan_start(datacall_status_callback);
	if(ret!=0)
	{
		ret = QL_GENERIC_FAILURE;
		goto exit;
	}
	ql_set_auto_connect(DATA_CALL_PROFILE_IDX, TRUE);
	//printf("start_data_call:%d\n", __LINE__);
	if(ret!=0)
	{
		ret = QL_GENERIC_FAILURE;
		goto exit;
	}

    for (i = 0; i < 5; i++)
    {
	    ret=ql_sim_get_imsi(IMSI, sizeof(IMSI));
        if (strlen(IMSI) != 0)
        {
            break;
        }
        ql_rtos_task_sleep_ms(1000);
    }

    ql_set_data_call_asyn_mode(1, ql_data_call_cb);

//    memset(PLMN, 0, sizeof(PLMN));
//    strncpy(PLMN, IMSI, 5);
	MODULE_GSM_INFO("%s_%d ===IMSI: %s\n", __func__, __LINE__, IMSI);
    pAPNinfo=GetApnInfo(IMSI);
   	if (pAPNinfo==NULL)
   	{
   		MODULE_GSM_DEBUG("%s_%d ===APN not found,use default\n", __func__, __LINE__);
   		pAPNinfo=&dApn;
   	}
	MODULE_GSM_DEBUG("%s_%d ===APN: %s\n", __func__, __LINE__,pAPNinfo->apn);
	ql_start_data_call(DATA_CALL_PROFILE_IDX, IP_PROTOCOL, pAPNinfo->apn, pAPNinfo->user, pAPNinfo->password, 0);
	//printf("start_data_call:%d\n", __LINE__);
	if(ret!=0)
		ret = QL_GENERIC_FAILURE;

exit:
	
	return ret;
}

int module_send_atcmd_sync(char *atcmd, char *resp, int resp_size)
{
    int ret = 0;
    int i = 0;
    char resp_buf[128] = {0};

    if (atcmd == NULL)
    {
        return -1;
    }

    for(i=0; i<3; i++)
	{
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync(atcmd, resp_buf, sizeof(resp_buf), NULL, 5);
		if(ret == 0)
		{
			break;
		}
	}

    if (resp != NULL)
    {
        strncpy(resp, resp_buf, resp_size);
    }
    
	return ret;
}

/*
导入fastdormancy，配置成15秒触发自动释放rrc，来规避没有收到基站释放RRC的消息，未释放RRC问题
*/
int open_fastdormancy_mode(void)
{
	int ret = 0;
    int i = 0;
    char resp_buf[128] = {0};
    int tmp_param = 0;
    int rrc_release_timeout = 0;

    for(i=0; i<3; i++)
    {
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync("AT+MEDCR=1,34,15\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
        if (ret == 0)
        {
            sscanf(resp_buf, "+MEDCR:%d,%d\r\n", &tmp_param, &rrc_release_timeout);
            MODULE_GSM_DEBUG("%s_%d ===rrc_release_timeout =%d\n", __func__, __LINE__, rrc_release_timeout);
            break;
        }
    }

    if ((ret == 0) && (rrc_release_timeout != 15))
    {
        for(i=0; i<3; i++)
    	{
            memset(resp_buf, 0, sizeof(resp_buf));
            ret = ql_atcmd_send_sync("AT+MEDCR=0,34,15\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
    		if(ret == 0)
    		{
    			break;
    		}
    	}
    }
    
	return ret;
}
int open_lteOptimize_mode(void)
{
	int ret = 0;
    int i = 0;
    char resp_buf[128] = {0};
    int tmp_param = 0;
    int rrc_release_timeout = 0;

    for(i=0; i<3; i++)
    {
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync("at+medcr=0,79,1\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
        if (ret == 0)
        {
            //sscanf(resp_buf, "+MEDCR:%d,%d\r\n", &tmp_param, &rrc_release_timeout);
            //MODULE_GSM_DEBUG("%s_%d ===rrc_release_timeout =%d\n", __func__, __LINE__, rrc_release_timeout);
            break;
        }
    }

    for(i=0; i<3; i++)
    {
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync("at+medcr=0,164,2\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
        if (ret == 0)
        {
            //sscanf(resp_buf, "+MEDCR:%d,%d\r\n", &tmp_param, &rrc_release_timeout);
            //MODULE_GSM_DEBUG("%s_%d ===rrc_release_timeout =%d\n", __func__, __LINE__, rrc_release_timeout);
            break;
        }
    }

    for(i=0; i<3; i++)
    {
        memset(resp_buf, 0, sizeof(resp_buf));
        ret = ql_atcmd_send_sync("at+medcr=0,160,1\r\n", resp_buf, sizeof(resp_buf), NULL, 5);
        if (ret == 0)
        {
            //sscanf(resp_buf, "+MEDCR:%d,%d\r\n", &tmp_param, &rrc_release_timeout);
            //MODULE_GSM_DEBUG("%s_%d ===rrc_release_timeout =%d\n", __func__, __LINE__, rrc_release_timeout);
            break;
        }
    }
    
	return ret;
}

int device_set_low_power_mode(void)
{
    module_send_atcmd_sync("at*pmicreg=w,1,25,81\r\n", NULL, 0);
    return 0;
}

int set_LTE_module_low_power(void)
{
    int ret ;
    //static int flag = 0; 

    //if (flag == 1) 
        //return 0;

    usb_log_printf("Enter ------ set_LTE_module_low_power func ------\n");
    
    ret = ql_atcmd_send_sync("at*pmicreg=w,1,25,81\r\n",NULL,0,NULL,5);
    if (ret != 0)
    {
        ret = ql_atcmd_send_sync("at*pmicreg=w,1,25,81\r\n",NULL,0,NULL,5);
        if (ret != 0) 
        {
            usb_log_printf("Failed : ql_atcmd_send_sync(at*pmicreg=w,1,25,81) \n");    
        }
    }
    
    ql_rtos_task_sleep_ms(100);
    
    ret = ql_atcmd_send_sync("at*pmicreg=w,1,24,88\r\n",NULL,0,NULL,5);
    if (ret != 0)
    {
        ret = ql_atcmd_send_sync("at*pmicreg=w,1,24,88\r\n",NULL,0,NULL,5);
        if (ret != 0) 
        {
            usb_log_printf("Failed : ql_atcmd_send_sync(at*pmicreg=w,1,24,88) \n");    
        }
    }

    //flag = 1;
    
    return ret;
}

int Module_Net_State(void)
{
	return data_call_successed_flag;
}

void module_net_task(void * argv)
{
	int ret = 0;
	uint16_t module_net_cnt = 280;
    struct ql_data_call_info info = {0};
	
	//检查SIM卡状态
	ret=check_sim_state();
	if(ret!=0)
	{
        g_network_register_status = 1;
		TermInfo.SIMState = 0;
		MODULE_GSM_ERROR("*** The sim is not available ! ***\n");
		goto exit;
//		return;
	}
	TermInfo.SIMState = 1;
	ModelInitOk = 1;
		
	//等待模块注上移动网络 180s
	ret=wait_network_register(180);
	if(ret>0)
	{
        g_network_register_status = 1;
		MODULE_GSM_ERROR("*** Network register failed ! ***\n");
		goto exit;
//		return;
	}
//    g_network_register_status = 1;
    MODULE_GSM_INFO("%s_%d === wait_for_read_sim_card ok\n", __func__, __LINE__);
    
#if 0
    ret = openAutoAnswer();

    if (ret == 0)
    {
        //tts_play_set(AudioOpenAutoAnswer,AudioOpenAutoAnswerLen,FixAudioTypeDef);
    }
#endif

	ql_nw_event_register(0x0f);
	ql_nw_add_event_handler(ql_nw_notify_cb, NULL);

	//启动模块数据拨号
	ret=start_data_call();
	if(ret>0)
	{
		MODULE_GSM_ERROR("*** Start data failed ! ***\n");
		goto exit;
//		return;
	}
    g_network_register_status = 1;
	MODULE_GSM_DEBUG("%s , line %d, module net init finished!\n", __func__, __LINE__);

	open_fastdormancy_mode();
	open_lteOptimize_mode();

    //device_set_low_power_mode();

//    set_LTE_module_low_power();
exit:    
	if(module_net_thread) 
	{
        module_net_thread = NULL;
        ql_rtos_task_delete(NULL);
	}
}

#if 1
int ModuleGetCellInfo(struct_location * location)
{
    char buf[64];
    QL_NW_CELL_INFO_T * ql_nw_cell_info=malloc(sizeof(QL_NW_CELL_INFO_T));
    int ret;
    int ii;
    int set;
    
    if (ql_nw_cell_info==NULL)
        return -1;

    ret=ql_nw_get_cell_info(ql_nw_cell_info);
    printf("%s,line %d, ret = %d\n", __func__,__LINE__,ret);

    set=0;
    if(ql_nw_cell_info->gsm_info_valid)
    {
        for(ii=0;ii<ql_nw_cell_info->gsm_info_num;ii++)
        {
            if (ql_nw_cell_info->gsm_info[ii].flag==0) 
            {
                set=ii;
                break;
            }
        }
        location->CID=ql_nw_cell_info->gsm_info[set].cid;
        location->LAC=ql_nw_cell_info->gsm_info[set].lac;
        location->MNC=ql_nw_cell_info->gsm_info[set].mnc;
        location->MCC=ql_nw_cell_info->gsm_info[set].mcc;
    }
    else if(ql_nw_cell_info->umts_info_valid)
    {
        for(ii=0;ii<ql_nw_cell_info->umts_info_num;ii++)
        {
            if (ql_nw_cell_info->gsm_info[ii].flag==0) 
            {
                set=ii;
                break;
            }
        }
        location->CID=ql_nw_cell_info->umts_info[set].cid;
        location->LAC=ql_nw_cell_info->umts_info[set].lac;
        location->MNC=ql_nw_cell_info->umts_info[set].mnc;
        location->MCC=ql_nw_cell_info->umts_info[set].mcc;
    }
    else if(ql_nw_cell_info->lte_info_valid)
    {
        for(ii=0;ii<ql_nw_cell_info->lte_info_num;ii++)
        {
            if (ql_nw_cell_info->gsm_info[ii].flag==0) 
            {
                set=ii;
                break;
            }
        }
        location->CID=ql_nw_cell_info->lte_info[set].cid;
        location->LAC=ql_nw_cell_info->lte_info[set].tac;
        location->MNC=ql_nw_cell_info->lte_info[set].mnc;
        location->MCC=ql_nw_cell_info->lte_info[set].mcc;
    }
    free(ql_nw_cell_info);
    
    return 0;
}
#else
int ModuleGetCellInfo(cJSON *root)
{
	char buf[64];
//	cJSON *root = NULL;
//	QL_NW_CELL_INFO_T  ql_nw_cell_infor;//=calloc(1,sizeof(QL_NW_CELL_INFO_T));
//	QL_NW_CELL_INFO_T * ql_nw_cell_info=&ql_nw_cell_infor;//=calloc(1,sizeof(QL_NW_CELL_INFO_T));
	QL_NW_CELL_INFO_T * ql_nw_cell_info=malloc(sizeof(QL_NW_CELL_INFO_T));
	int ret;
	int ii;
	int set;
	
	if (ql_nw_cell_info==NULL)
		return -1;

//	root = cJSON_CreateObject();
//	if (root==NULL) 
//		goto cJson_EXIT;

	ret=ql_nw_get_cell_info(ql_nw_cell_info);
	MODULE_GSM_INFO("ql_nw_get_ql_nw_cell_info ret = %d\n", ret);

	set=0;
	if(ql_nw_cell_info->gsm_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->gsm_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [GSM] cid:%d, mcc:%d, mnc:%d, lac:%d, arfcn:%d, bsic:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->gsm_info[i].cid,
//				ql_nw_cell_info->gsm_info[i].mcc,
//				ql_nw_cell_info->gsm_info[i].mnc,
//				ql_nw_cell_info->gsm_info[i].lac,
//				ql_nw_cell_info->gsm_info[i].arfcn,
//				ql_nw_cell_info->gsm_info[i].bsic,
//				ql_nw_cell_info->gsm_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].lac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->gsm_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->gsm_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
	else if(ql_nw_cell_info->umts_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->umts_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [UMTS] cid:%d, lcid:%d, mcc:%d, mnc:%d, lac:%d, uarfcn:%d, psc:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->umts_info[i].cid,
//				ql_nw_cell_info->umts_info[i].lcid,
//				ql_nw_cell_info->umts_info[i].mcc,
//				ql_nw_cell_info->umts_info[i].mnc,
//				ql_nw_cell_info->umts_info[i].lac,
//				ql_nw_cell_info->umts_info[i].uarfcn,
//				ql_nw_cell_info->umts_info[i].psc,
//				ql_nw_cell_info->umts_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].lac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->umts_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->umts_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
	else if(ql_nw_cell_info->lte_info_valid)
	{
		for(ii=0;ii<ql_nw_cell_info->lte_info_num;ii++)
		{
			if (ql_nw_cell_info->gsm_info[ii].flag==0) 
			{
				set=ii;
				break;
			}
//			printf("Cell_%d [LTE] cid:%d, mcc:%d, mnc:%d, tac:%d, pci:%d, earfcn:%d, rssi:%d\n",
//				i,
//				ql_nw_cell_info->lte_info[i].cid,
//				ql_nw_cell_info->lte_info[i].mcc,
//				ql_nw_cell_info->lte_info[i].mnc,
//				ql_nw_cell_info->lte_info[i].tac,
//				ql_nw_cell_info->lte_info[i].pci,
//				ql_nw_cell_info->lte_info[i].earfcn,
//				ql_nw_cell_info->lte_info[i].rssi);
		}
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].tac);
		cJSON_AddItemToObject(root, "lac", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].cid);
		cJSON_AddItemToObject(root, "cell", cJSON_CreateString(buf));
		sprintf(buf,"%x",ql_nw_cell_info->lte_info[set].mcc);
		cJSON_AddItemToObject(root, "mcc", cJSON_CreateString(buf));
		sprintf(buf,"%d",ql_nw_cell_info->lte_info[set].mnc);
		cJSON_AddItemToObject(root, "mnc", cJSON_CreateString(buf));
	}
	else
		MODULE_GSM_INFO("cell info invalid !\n", ret);
		
cJson_EXIT:
	free(ql_nw_cell_info);  
	
	return 0;
}
#endif

int ModelInitResult(void)
{
	return ModelInitOk;
}

int GetGsmSignalLevel(void)
{
    QL_NW_SIGNAL_STRENGTH_INFO_T signal_info;

    ql_nw_get_signal_strength(&signal_info);
    return signal_info.LTE_SignalStrength.rsrp;
}

int GetLteIp(uint8_t *buff,uint8_t buffsize)
{
    int ret;
    struct ql_data_call_info info = {0};
    char ip_addr_str[64] = {0};
    ret = ql_get_data_call_info(DATA_CALL_PROFILE_IDX, IP_PROTOCOL, &info);
    if ( ret == 0 )
    {
        if ( info.ip_version )
        {
            inet_ntop( AF_INET6, &info.v6.addr.ip, ip_addr_str, sizeof(ip_addr_str) );
        }
        else
        {

            inet_ntop( AF_INET, &info.v4.addr.ip, ip_addr_str, sizeof(ip_addr_str) );
        }
        strncpy(buff,ip_addr_str,buffsize);
    }
    return ret;
}

void LTE_CAT1_init(void)
{
	if (ql_rtos_task_create(&module_net_thread,
						3*1024,
						100,
						"module_net_task",
						module_net_task,
						NULL) != OS_OK) {
	}

	MODULE_GSM_DEBUG("enter %s , line %d\n", __func__, __LINE__);
}
