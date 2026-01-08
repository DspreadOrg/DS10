#ifndef __DRV_BAT_H__
#define __DRV_BAT_H__

#ifdef __cplusplus
	 extern "C" {
#endif

typedef enum{
	ChargeInsert,
	ChargeRemove,
	ChargeOver,
	BatCapChange,
}ChargeEvent;

typedef struct {
	short vol;
	short per;
}BatCapDef; 



extern void bat_chrg_initCapTable(short const * chg_ptable,int chg_ItemSize,short const * dischg_ptable,int dischg_ItemSize);

/**
  * @brief The status of chrg interrupt.
  */

/**
 * @brief  软件检测充电状态
 * @param  batFullVol - 满电量参考充电电流阀值(mA)
 * @param  FullTimeSec - 满电量参考充电电流阀值以下持续时间
 * @retval 0：设置成功，小于0：充电电流功能不支持
 * @note   none
 */

int bat_full_current_set(unsigned int batFullCurrent,int FullTimeSec);
/**
 * @brief  软件检测充电状态
 * @param  batFullVol - 满电量参考电压阀值(mV)
 * @param  FullTimeSec - 满电量参考电压阀值以上持续时间
 * @retval none
 * @note   none
 */
//void bat_full_soft_counter(unsigned short curBatVal, unsigned int batFullVol );
void bat_full_vol_set(unsigned int batFullVol,int FullTimeSec);
/**
 * @brief  软件检测电压刷新
 * @param  curBatVal - 当前电压
 * @retval none
 * @note   none
 */
//void bat_full_vol_feed(unsigned int curBatVal);

/**
 * @brief  充电满检测
 * @param  chkNumMax - 软件检测满电阀值（wifi模式有效）
 * @retval 0 - 没满或没充电，1 - 充电满
 * @note   none
 */
int bat_full_check(void);

/**
 * @brief  充电电流检测
 * @retval 大于0，单位mA，小于0不支持或读失败
 * @note   none
 */
int bat_charg_current(void);

/**
 * @brief  充电是否满
 * @param  none
 * @retval 0 - 没满或没充电，1 - 充电满
 * @note   none
 */
int is_charge_full(void);

/**
 * @brief  获取充电状态
 * @param  none
 * @retval 0 - 充电移出，1 - 充电中
 * @note   none
 */
int get_charge_status(void);

///**
// * @brief  充电控制开关
// * @param  none
// * @retval 0 - 充电打开，1 - 充电关闭
// * @note   none
// */
//void bat_Charge_OnOff(int OnOff);

/**
 * @brief  充电功能初始化
 * @param  none
 * @retval none
 * @note   none
 */
void bat_chrg_init(void);

#ifdef __cplusplus
	 }
#endif

#endif

