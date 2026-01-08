
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/


#ifndef __QL_USB_H__
#define __QL_USB_H__


//states: 1>USB插入 0>USB拔出
typedef void (*usbdectcb)(unsigned int states);

/*************************************************
*注册USB插入与拔出的检测回调函敿
*函数成功返回0，否则返囿1
/************************************************/
int ql_usbdect_register_cb(usbdectcb cb);


/*****************************************************************
* Function: ql_usb_get_type
*
* Description:
* 	获取USB的类型。 
* 
* Parameters:
* 	void	  		  
* Return:
* 	1				USB正常枚举。
*	0		        USB未能正常枚举,非USB,可能是电源适配器。
*
NOTES:此接口耗时2s
*****************************************************************/
extern int ql_usb_get_type(void);


/*****************************************************************
* Function: ql_usb_connect_state
*
* Description:
* 	获取USB的连接状态,d+,d-。 [!!!此接口目前有bug,不建议使用@2022/04/26]
* 
* Parameters:
* 	void	  		  
* Return:
* 	1				USB正常枚举且可以正常通讯。
*	0		        USB未连接。
*
*****************************************************************/
int ql_usb_connect_state(void);

/*****************************************************************
* Function: ql_usb_plugin
*
* Description: 软件触发插入USB,进行枚举。
* 
* Parameters: void	  		  
* Return: void
*****************************************************************/
void ql_usb_plugin(void);

/*****************************************************************
* Function: ql_usb_unplug
*
* Description: 软件触发拔出USB,进行去枚举。
* 
* Parameters: void	  		  
* Return: void
*****************************************************************/
void ql_usb_unplug(void);

/*****************************************************************
* Function: ql_usb_disable_enumeration
*
* Description:
* 	关闭USB枚举，配置掉电保存。 
* 
* Parameters:
* 	disable   是否关闭usb枚举，1 关闭 ，0 打开
* Return:
*   1				  配置成功
*	  0		      配置失败
*
*****************************************************************/
int ql_usb_disable_enumeration(unsigned int disable);


/*****************************************************************
* Function: ql_cdc_switch_ccid_desc
*
* Description:
* 	将cdc接口切换为ccid标准设备。 
* 
* Parameters:
* 	enable:   是否将cdc切换为ccid，1 ccid ，0 cdc
* Return:
*   void:
*
*****************************************************************/
void ql_cdc_switch_ccid_desc(unsigned char enable);

//USB网卡绑定
typedef enum 
{
    QL_NETIF_DISCONNECT=0,
    QL_NETIF_CONNECT,
    QL_NETIF_REMOVE=0xFF,
}ql_netif_status_e;
int ql_netdevctl_lwip_config(ql_netif_status_e netif_config,uint8 set_cid);


#endif


