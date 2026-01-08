#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ql_rtos.h"
#include "ql_power.h"

#include "prj_common.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "public_api_interface.h"
#include "tts_yt_task.h"
#include "disp_port.h"
#include "drv_bat.h"

//#define BATTERY_CHARGE_MMPT_SUPPORT      0

#define BAT_DEBUG(fmt, arg...) //printf("[[DBG DRV_BAT]"fmt, ##arg)
#define BAT_INFO(fmt, arg...) //usb_log_printf("[INFO DRV_BAT]"fmt, ##arg)

//#define VOLTEST
#ifdef VOLTEST
#define BATTERY_CC_START_VOL     3900
#define BATTERY_FULL_VOL        4000
#define BATTERY_FULL_CURRENT    200

#define BATTERY_FULL_SOFT_CHECK_SEC      (1200)
//static BatCapDef discharg_table[]=
//{
//    {4550,100},
//    {4350,10},//低电阀值
//    {4250,0},//关机阀值
//};
#else
//浮充重新充电阀值，无浮充功能硬件上一直在充电状态电压不会跌落
#define BATTERY_CC_START_VOL     3980//4000
//充电判断满电压监测阀值，高于该值启动计时，解决有些WIFI状态充电硬件一直无充电满硬件信号问题。电压值再高软件满功能可能失效
#define BATTERY_FULL_VOL        4150
//充电判断满电流监测阀值，低于该值启动计时，无浮充功能电路会一直高于该值。
#define BATTERY_FULL_CURRENT    350

//满电压计时时间，预留30分钟硬件继续充电，解决阀值后充电电流还比较大的情况。
#define BATTERY_FULL_VOL_CHECK_SEC      (60*45)
//满电流计时时间，满电流阀值后充电基本完成
#define BATTERY_FULL_CURRENT_CHECK_SEC      (90)

#endif

//充电表
static BatCapDef charg_table[]=
{
	{4165,100},		//	{4165,100},
	{4140,90},		//    {4130,90},
	{4115,80},		//    {4060,80},
	{4090,70},		//    {4015,70},
	{4070,60},		//    {4005,60},
	{4035,50},		//    {3965,50},
	{3985,40},		//    {3945,40},
	{3940,30},		//    {3915,30},
	{3875,20},		//    {3875,20},
	{3800,10},		//    {3800,10},//低电阀值
	{3530,0},		//    {3530,0},//关机阀值
};
//放电表
static BatCapDef discharg_table[]=
{
	{4070,100},		//	{4100,100},
	{4040,90},		//    {4030,90},
	{4015,80},		//    {3960,80},
	{3980,70},		//    {3910,70},
	{3960,60},		//    {3880,60},
	{3930,50},		//    {3855,50},
	{3880,40},		//    {3830,40},
	{3830,30},		//    {3800,30},
	{3760,20},		//    {3760,20},
	{3550,10},		//    {3550,10},//低电阀值
	{3500,0},		//    {3400,0},//关机阀值
};

extern void bat_chrg_init();


void app_set_battery_status(int status)
{
//	BAT_INFO("[%s_%d]:status %d=======================\n", __func__, __LINE__, status);
	TermInfo.LowBat = status;
}


void ChargeEventCb(ChargeEvent event)
{
	int per;
	
	switch(event)
	{
		case ChargeInsert:
			//tts_play_set_idx(AUD_ID_CHARGE_IN,0,0);
			//app_set_battery_status(0);
			ChargeStatLedSet(1);
			break;
		case ChargeRemove:
			//tts_play_set_idx(AUD_ID_CHARGE_OUT,0,0);
			//per=battery_read_percent();
			//if ((per>0)&&(per<10))
			//	app_set_battery_status(1);
			//else if (per==0)
			//	app_set_battery_status(2);
			ChargeStatLedSet(0);
			break;
		case ChargeOver:
			//tts_play_set_idx(AUD_ID_CHARGE_FULL,0,0);
			ChargeStatLedSet(0);
			break;
		case BatCapChange:
			//disp_update_request();
			per=battery_read_percent();
			//if (get_charge_status())
			//	app_set_battery_status(0);
			BAT_INFO("[%s_%d]:================ bat per change,cur %d!\n", __func__, __LINE__,per);
			if (per>=10)
				app_set_battery_status(0);
			else if (per>=0)
			{
				if (battery_lowpwr())
					app_set_battery_status(2);
				else
					app_set_battery_status(1);
			}

			break;
	}
}

void drv_bat_init(void)
{
	int per;
	//电量表初始化
	bat_chrg_initCapTable(charg_table,sizeof(charg_table)/sizeof(charg_table[0]),discharg_table,sizeof(discharg_table)/sizeof(discharg_table[0]));
	//充电满电电流阀值设置，充电电流检测功能启用
	if (bat_full_current_set(BATTERY_FULL_CURRENT,BATTERY_FULL_CURRENT_CHECK_SEC)<0)
	{
		BAT_INFO("[%s_%d]: charge current not surport!\n", __func__, __LINE__);
	}
	//充电满电电压阀值设置，充电电压检测功能启用
	bat_full_vol_set(BATTERY_FULL_VOL,BATTERY_FULL_VOL_CHECK_SEC);
	//浮充时，重新充电电压阀值设置
	bat_set_rechargeVol(BATTERY_CC_START_VOL);
	//电池电压状态回调
	set_charge_event_cb(ChargeEventCb);
	//电池监测线程处理启用
	bat_chrg_init();
}
