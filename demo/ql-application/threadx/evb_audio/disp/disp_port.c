/**
  ******************************************************************************
  * @file    disp_port.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   display port func file for C code
  * @code    GBK编码，
  ******************************************************************************
  * @attention
  * <h2><center>&copy; COPYRIGHT 2021 YMZN</center></h2>
  ******************************************************************************
  */
 
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_spi.h"
#include "ql_pwm.h"
#include "ql_gpio.h"
#include "ql_power.h"
#include "ql_rtc.h"
#include "prj_common.h"
#include "systemparam.h"
#include "terminfodef.h"
#include "tts_yt_task.h"
#include "gpio.h"
#include "lv_port_disp.h"
#include "animate.h"
#include "disp_port.h"

#define LOG_DBG(...)            //do{printf("[DISP DBG]: "); printf(__VA_ARGS__);}while(0)
#define LOG_INFO(...)           do{printf("[DISP INFO]: "); printf(__VA_ARGS__);}while(0)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


// display init
void disp_port_init(void)
{
    const dev_config_t *pdevconf = get_device_config();

#if DEV_FG7864_SUPPORT
    if( pdevconf->opt->bits.has_lcd_fg7864 )
    {
        // fg7864 init
        LOG_INFO("%s: add disp-fg7864 support\n",__func__);
        TermInfo.disp.fg7864 = 1;
        lcd_init();
    }
#elif DEV_F240320_SUPPORT 
    if( pdevconf->pins->DISP_DEF.opt.has_lcd_f240320 )
    {
        // f240320 init
        LOG_INFO("%s: add disp-f240320 support\n",__func__);
        TermInfo.disp.f240320 = 1;
#if DEV_KEYBOARD_SUPPORT
        TermInfo.disp.tm1721 = 1;
#endif
        lcd_f240320_init();
    }
#elif DEV_F480320_SUPPORT
    if( pdevconf->pins->DISP_DEF.opt.has_lcd_f240320 )
    {
        // f240320 init
        LOG_INFO("%s: add disp-f240320 support\n",__func__);
        TermInfo.disp.f240320 = 1;
#if DEV_KEYBOARD_SUPPORT
        TermInfo.disp.tm1721 = 1;
#endif
        lcd_f480320_init();
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( pdevconf->pins->DISP_DEF.opt.has_led_fg00ahk )
    {
        // fg00ahk init
        LOG_INFO( "%s: add disp-fg00ahk support\n", __func__ );
        TermInfo.disp.fg00ahk = 1;
#ifndef DEV_FG00AHK_TYPE
        led_fg00_init();
#else
  #if   DEV_FG00AHK_TYPE == DEV_FG00AHK_0964
        led_fg00_0964_init();
  #elif DEV_FG00AHK_TYPE == DEV_FG00AHK_1921
        led_fm001291_init();
  #elif DEV_FG00AHK_TYPE == DEV_FG00AHK_1921B
        led_fm001291B_init();
  #else
     #error "DEV_FG00AHK_TYPE not match!"
  #endif        
#endif        
    }
#endif

#if DEV_TM1604_SUPPORT
    if( pdevconf->opt->bits.has_led_tm1604 )
    {
        // tm1604 init
        LOG_INFO("%s: add disp-tm1604 support\n",__func__);
        TermInfo.disp.tm1604 = 1;
        lcd_task_init();
    }
#endif
	disp_set_backlight(100);
}

/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsReady
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static bool LvglPortingCallbackFsReady(struct _lv_fs_drv_t * drv)
{
	return true;
}


/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsOpen
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static void * LvglPortingCallbackFsOpen(struct _lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
	int res = -1;
	char m[8] = {0};
	static QFILE* fp = NULL;
	if(mode == LV_FS_MODE_WR)
	{
		strcpy(m,"w+");
	}
	else
	{
		strcpy(m,"rb");
	}

	fp = ql_fopen( path, m);

	return fp;
}


/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsClose
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static lv_fs_res_t LvglPortingCallbackFsClose(struct _lv_fs_drv_t * drv, void * file_p)
{
	int res = -1;
	QFILE* fp = file_p;

	res = ql_fclose(fp);

	if(res != 0)
	{
		return LV_FS_RES_UNKNOWN;
	}
	else
	{
		return LV_FS_RES_OK;
	}
}


/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsRead
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static lv_fs_res_t LvglPortingCallbackFsRead(struct _lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
	int res = -1;
	QFILE* fp = file_p;

	res = ql_fread(buf, btr, 1,fp );
	if(res < 0)
	{
        *br = 0;
		return LV_FS_RES_UNKNOWN;
	}
	else
	{
        *br = res;
		return LV_FS_RES_OK;
	}
}


/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsWrite
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static lv_fs_res_t LvglPortingCallbackFsWrite(struct _lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
	int res = -1;
    QFILE* fp = file_p;

	res = ql_fwrite(buf, btw, 1,fp);
	if(res < 0)
	{
        *bw = 0;
		return LV_FS_RES_UNKNOWN;
	}
	else
	{
        *bw = res;
		return LV_FS_RES_OK;
	}
}


/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsSeek
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static lv_fs_res_t LvglPortingCallbackFsSeek(struct _lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
	int res = -1;
    QFILE* fp = file_p;

	if(whence == LV_FS_SEEK_CUR)
	{
		pos += ql_ftell(fp);
	}
	else if(whence == LV_FS_SEEK_END)
	{
		pos = ql_fsize(fp) - pos;
	}

	res = ql_fseek(fp, pos,SEEK_SET);
	if(res < 0)
	{
		return LV_FS_RES_UNKNOWN;
	}
	else
	{
		return LV_FS_RES_OK;
	}
}

/*--------------------------------------
|	Function Name:
|		LvglPortingCallbackFsTell
|	Description:
|	Parameters:
|	Returns:
+-------------------------------------*/
static lv_fs_res_t LvglPortingCallbackFsTell(struct _lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    QFILE* fp = file_p;

	*pos_p = ql_ftell(fp);

	return LV_FS_RES_OK;
}

static void LvglPortingCallbackLog(const char *buf)
{
	usb_log_printf("%s \n", buf);
}
void lv_fs_init()
{
		static lv_fs_drv_t fs_drv;

		lv_fs_drv_init(&fs_drv);
		fs_drv.letter = 'B';
		fs_drv.ready_cb = LvglPortingCallbackFsReady;
		fs_drv.open_cb = LvglPortingCallbackFsOpen;
		fs_drv.close_cb = LvglPortingCallbackFsClose;
		fs_drv.read_cb = LvglPortingCallbackFsRead;
		fs_drv.write_cb = LvglPortingCallbackFsWrite;
		fs_drv.seek_cb = LvglPortingCallbackFsSeek;
		fs_drv.tell_cb = LvglPortingCallbackFsTell;
		fs_drv.dir_open_cb = NULL;
		fs_drv.dir_read_cb = NULL;
		fs_drv.dir_close_cb = NULL;
		fs_drv.user_data = NULL;
		lv_fs_drv_register(&fs_drv);

        #if LV_USE_LOG
        lv_log_register_print_cb(LvglPortingCallbackLog);
        #endif
}
// disp high level ui init
void disp_ui_init(void)
{
    LOG_DBG("%s: call\n",__func__);

#if DEV_FG7864_SUPPORT
    int vol = sysparam_get( )->volume;
    uint8_t level;
    if( TermInfo.disp.fg7864 )
    {
        // ICON init
        lcd_set_icon(ICON_BAT_OUTLINE,1);
        lcd_set_icon(ICON_VOL_OUTLINE,1);

        lcd_set_vol_level(level);

        // lvgl init
        lv_init();
        // disp lv port init
        lv_port_disp_init();
        // ui init
        ui_init();
        // anm init
        animate_task_init();
    }
#elif DEV_F240320_SUPPORT || DEV_F480320_SUPPORT
    if( TermInfo.disp.f240320 )
    {
        // lvgl init
        lv_init();
        //lvgl file system init
        lv_fs_init();
        // disp lv port init
        lv_port_disp_init();
        // ui init
        ui_init();
        // anm init
        animate_task_init();
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        g_fg00_info.speaker = 1;
        g_fg00_info.yuan = 1;
        g_fg00_info.bat = 1;
        g_fg00_info.speaker_level_1 = 1;
        g_fg00_info.speaker_level_2 = 1;
        g_fg00_info.g_num = FG00_CHAR_BLANK;

        memset(g_fg00_info.s_num,FG00_CHAR_BLANK,LED_S_NUM_COUNT);
        memset(g_fg00_info.e_num,FG00_CHAR_BLANK,LED_E_NUM_COUNT);
        led_fg00_show_string(FG00_EVENT_MSTR,LED_FG00_BLANK_STR,5000);
        disp_vol_update(0);
        animate_task_init();
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
    }
#endif
}


// disp power off
void disp_poweroff_msg(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_TM1604_SUPPORT
    if ( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(DIS_PWR_OFF, 0, PWR_OFF_SHOW);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        led_fg00_show_string(FG00_EVENT_MSTR,LED_FG00_OFF_STR,5000);
    }
#endif

#if DEV_KEYBOARD_SUPPORT
    if( TermInfo.disp.tm1721 )
    {
        keyboard_onoff_show(0);
    }
#endif
}

// disp none mode
void disp_none_mode(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_icon(ICON_WIFI_ERROR,0);
        lcd_set_icon(ICON_WIFI_OUTLINE,0);

        lcd_set_icon(ICON_GPRS_ERROR,0);
        lcd_set_icon(ICON_GPRS_UNDERLINE,0);
        lcd_set_icon(ICON_GPRS_2G,0);
        lcd_set_icon(ICON_GPRS_4G,0);
        lcd_set_icon(ICON_GPRS_LEVEL_1,0);
        lcd_set_icon(ICON_GPRS_LEVEL_2,0);
        lcd_set_icon(ICON_GPRS_LEVEL_3,0);
        lcd_set_icon(ICON_GPRS_LEVEL_4,0);

        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        g_fg00_info.g_num = FG00_CHAR_BLANK;
        g_fg00_info.gprs_err = 0;
        g_fg00_info.gprs_g = 0;
        g_fg00_info.gprs_level_1 = 0;
        g_fg00_info.gprs_level_2 = 0;
        g_fg00_info.gprs_level_3 = 0;
        g_fg00_info.gprs_level_4 = 0;

        g_fg00_info.wifi_err = 0;
        g_fg00_info.wifi_level_1 = 0;
        g_fg00_info.wifi_level_2 = 0;
        g_fg00_info.wifi_level_3 = 0;
        g_fg00_info.wifi_level_4 = 0;
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif
}

// disp switch to gprs mode
void disp_gprs_mode(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_icon(ICON_WIFI_ERROR,0);
        lcd_set_icon(ICON_WIFI_OUTLINE,0);

        lcd_set_icon(ICON_GPRS_ERROR,0);
        lcd_set_icon(ICON_GPRS_UNDERLINE,0);
        lcd_set_icon(ICON_GPRS_2G,0);
        lcd_set_icon(ICON_GPRS_4G,1);
        lcd_set_icon(ICON_GPRS_LEVEL_1,0);
        lcd_set_icon(ICON_GPRS_LEVEL_2,0);
        lcd_set_icon(ICON_GPRS_LEVEL_3,0);
        lcd_set_icon(ICON_GPRS_LEVEL_4,0);
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        g_fg00_info.gprs_err = 0;
        g_fg00_info.g_num = 4;
        g_fg00_info.gprs_g = 1;
        g_fg00_info.gprs_level_1 = 0;
        g_fg00_info.gprs_level_2 = 0;
        g_fg00_info.gprs_level_3 = 0;
        g_fg00_info.gprs_level_4 = 0;

        g_fg00_info.wifi_err = 0;
        g_fg00_info.wifi_level_1 = 0;
        g_fg00_info.wifi_level_2 = 0;
        g_fg00_info.wifi_level_3 = 0;
        g_fg00_info.wifi_level_4 = 0;
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_KEYBOARD_SUPPORT
    if( TermInfo.disp.tm1721 )
    {
        keyboard_set_mode(1);
    }
#endif
}

// disp switch gprs state
void disp_set_gprs_error( uint8_t is_err )
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        if ( is_err )
        {
            lcd_set_icon( ICON_GPRS_ERROR, 1 );
        }
        else
        {
            lcd_set_icon( ICON_GPRS_ERROR, 0 );
        }
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        if( is_err )
        {
            g_fg00_info.gprs_err = 1;
        }
        else
        {
            g_fg00_info.gprs_err = 0;
        }
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_TM1604_SUPPORT
    if ( TermInfo.disp.tm1604 )
    {
        if ( is_err )
        {
            release_lcd_queue( DISTRI_NET_FAIL, 0, NULL );
        }
        else
        {
            release_lcd_queue( DISTRI_NET_SUCCES, 0, NULL );
        }
    }
#endif
}

// disp_set gprs signal level
void disp_set_gprs_signal_level(uint8_t level)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_gprs_level(level);
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        fg00_set_gprs_level(level);
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_KEYBOARD_SUPPORT
    if( TermInfo.disp.tm1721 )
    {
        keyboard_set_signal(level);
    }
#endif
}

// disp_set wifi signal level
void disp_set_wifi_signal_level(uint8_t level)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        // not support
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        fg00_set_wifi_level(level);
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_KEYBOARD_SUPPORT
    if( TermInfo.disp.tm1721 )
    {
        keyboard_set_signal(level);
    }
#endif
}

// disp switch to wifi mode
void disp_wifi_mode(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_icon(ICON_GPRS_ERROR,0);
        lcd_set_icon(ICON_GPRS_UNDERLINE,0);
        lcd_set_icon(ICON_GPRS_2G,0);
        lcd_set_icon(ICON_GPRS_4G,0);
        lcd_set_icon(ICON_GPRS_LEVEL_1,0);
        lcd_set_icon(ICON_GPRS_LEVEL_2,0);
        lcd_set_icon(ICON_GPRS_LEVEL_3,0);
        lcd_set_icon(ICON_GPRS_LEVEL_4,0);

        lcd_set_icon(ICON_WIFI_ERROR,0);
        lcd_set_icon(ICON_WIFI_OUTLINE,1);
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        g_fg00_info.g_num = FG00_CHAR_BLANK;
        g_fg00_info.gprs_err = 0;
        g_fg00_info.gprs_g = 0;
        g_fg00_info.gprs_level_1 = 0;
        g_fg00_info.gprs_level_2 = 0;
        g_fg00_info.gprs_level_3 = 0;
        g_fg00_info.gprs_level_4 = 0;

        g_fg00_info.wifi_err = 0;
        g_fg00_info.wifi_level_1 = 1;
        g_fg00_info.wifi_level_2 = 1;
        g_fg00_info.wifi_level_3 = 1;
        g_fg00_info.wifi_level_4 = 1;
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_KEYBOARD_SUPPORT
    if( TermInfo.disp.tm1721 )
    {
        keyboard_set_mode(2);
    }
#endif
}

// disp switch wifi state
void disp_set_wifi_error(uint8_t is_err)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        if( is_err )
        {
           lcd_set_icon(ICON_WIFI_ERROR,1);
        }
        else
        {
            lcd_set_icon(ICON_WIFI_ERROR,0);
        }
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        if( is_err )
        {
            g_fg00_info.wifi_err = 1;
        }
        else
        {
            g_fg00_info.wifi_err = 0;
        }
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        if( is_err )
        {
            release_lcd_queue(DISTRI_NET_FAIL, 0, NULL);
        }
        else
        {
            release_lcd_queue(DISTRI_NET_SUCCES, 0, NULL);
        }
    }
#endif
}

// disp up/down state
void disp_set_updown_state(uint8_t state)
{
    LOG_DBG("%s: call\n",__func__);
 #if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        if( state == 1 )
        {
            // up flag
            lcd_set_icon(ICON_UP,1);
            lcd_set_icon(ICON_DOWN,0);
        }
        else if( state == 2 )
        {
            // down flag
            lcd_set_icon(ICON_UP,0);
            lcd_set_icon(ICON_DOWN,1);
        }
        else if( state == 3 )
        {
            // up/down flag
            lcd_set_icon(ICON_UP,1);
            lcd_set_icon(ICON_DOWN,1);
        }
        else
        {
            // hide up/down flag
            lcd_set_icon(ICON_UP,0);
            lcd_set_icon(ICON_DOWN,0);
        }

        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif
}

// disp update vol
void disp_vol_update(uint8_t is_show_vol)
{
    LOG_DBG("%s: call\n",__func__);
    int level = sysparam_get( )->volume;
//    uint8_t level = 1;
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_vol_level(level);
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
//    	level=vol+1;
//		if(get_device_config()->id==ID_CS62CK)
//        fg00_set_vol_level(level-1);
//        else
        fg00_set_vol_level(level);
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
        if ( is_show_vol )
        {
            char vol_str[10] = { 0, };
            snprintf( vol_str, sizeof( vol_str ), " -%02d- ", level );
            led_fg00_show_string( FG00_EVENT_MSTR, vol_str, 10 * 1000 );
        }
    }
#endif

#if DEV_TM1604_SUPPORT
    if ( TermInfo.disp.tm1604 )
    {
        volume_num_to_show(level);
    }
#endif
}

// set battery level
void disp_set_bat_level(uint8_t level)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_bat_level(level);
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        fg00_set_bat_level(level);
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif
}

// set backlight level
void disp_set_backlight(uint8_t level )
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        lcd_set_bl_level(level);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        led_fg00_set_bl_level(level);
    }
#endif
}

// disp ota state
void disp_set_ota_state(int percent)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {

    }
#endif
#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        if( percent == 0 )
        {
            // ota start
            release_lcd_queue(DEVICE_OTA, 0, NULL);
        }
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        if ( percent == -1 )
        {
            //g_fg00_info.yuan = 0;
            fg00_send_event( FG00_EVENT_BUSY, 0, NULL );
        }
        else if ( percent >= 0 )
        {
            //g_fg00_info.yuan = 0;
            char percent_str[7] = { 0, };
            snprintf( percent_str, sizeof( percent_str ), "%d", percent );
            disp_set_led_msg( percent_str, DISP_HOLDON_FOREVER );
        }
        else
        {
            disp_state_idle( );
        }
    }
#endif
}

// disp network connecting
void disp_net_connecting(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(CONNECT_NET, 0, NULL);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        fg00_send_event(FG00_EVENT_NET_CONN,0,NULL);
    }
#endif
}

// disp service connecting
void disp_service_connecting(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(CONNECT_NET, 0, NULL);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
     {
         fg00_send_event(FG00_EVENT_SVC_CONN,0,NULL);
     }
#endif
}

// disp gprs connect state
void disp_set_net_connect_error(uint8_t is_err)
{
    LOG_DBG("%s: call\n",__func__);

#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        if ( ( TermInfo.NetMode == GPRS_MODE ) || ( TermInfo.NetMode == GPRS_BAKE_MODE ) )
        {
            if ( is_err )
            {
                lcd_set_icon( ICON_GPRS_ERROR, 1 );
                if ( TermInfo.SIMState != 1 )
                {
                    // no signal
                    lcd_set_icon( ICON_GPRS_LEVEL_1, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_2, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_3, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_4, 0 );
                }
            }
            else
            {
                lcd_set_icon( ICON_GPRS_ERROR, 0 );
            }
            lcd_frame_update( FRAME_FLAG_ICON );
        }
        else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
        {
            if( is_err )
            {
               lcd_set_icon(ICON_WIFI_ERROR,1);
            }
            else
            {
                lcd_set_icon(ICON_WIFI_ERROR,0);
            }
            lcd_frame_update( FRAME_FLAG_ICON );
        }
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        if ( ( TermInfo.NetMode == GPRS_MODE ) || ( TermInfo.NetMode == GPRS_BAKE_MODE ) )
        {
            if( is_err )
            {
                g_fg00_info.gprs_err = 1;
                if ( TermInfo.SIMState != 1 )
                {
                    g_fg00_info.gprs_level_1 = 0;
                    g_fg00_info.gprs_level_2 = 0;
                    g_fg00_info.gprs_level_3 = 0;
                    g_fg00_info.gprs_level_4 = 0;
                }
                led_fg00_show_string(FG00_EVENT_MSTR,"Err   ",DISP_HOLDON_FOREVER);
            }
            else
            {
                g_fg00_info.gprs_err = 0;
                led_fg00_show_string(FG00_EVENT_MSTR,LED_FG00_BLANK_STR,10 * 1000);
            }
        }
        else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
        {
            if( is_err )
            {
                g_fg00_info.wifi_err = 1;
                led_fg00_show_string(FG00_EVENT_MSTR,"Err   ",DISP_HOLDON_FOREVER);
            }
            else
            {
                g_fg00_info.wifi_err = 0;
                led_fg00_show_string(FG00_EVENT_MSTR,LED_FG00_BLANK_STR,10*1000);
            }
        }
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        if( is_err )
        {
            release_lcd_queue(CONNECT_NET_FAIL, 0, NULL);
        }
        else
        {
            release_lcd_queue(CONNECT_NET_SUCCES, 0, NULL);
        }
    }
#endif
}
// disp set server connect state
void disp_set_service_connect_error(uint8_t is_err)
{
    LOG_DBG("%s: is_err=%d\n",__func__,is_err);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        if ( ( TermInfo.NetMode == GPRS_MODE ) || ( TermInfo.NetMode == GPRS_BAKE_MODE ) )
        {
            if ( is_err )
            {
                lcd_set_icon( ICON_GPRS_ERROR, 1 );
                if ( TermInfo.SIMState != 1 )
                {
                    // no signal
                    lcd_set_icon( ICON_GPRS_LEVEL_1, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_2, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_3, 0 );
                    lcd_set_icon( ICON_GPRS_LEVEL_4, 0 );
                }
            }
            else
            {
                lcd_set_icon( ICON_GPRS_ERROR, 0 );
            }
            lcd_frame_update( FRAME_FLAG_ICON );
        }
        else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
        {
            if( is_err )
            {
               lcd_set_icon(ICON_WIFI_ERROR,1);
            }
            else
            {
                lcd_set_icon(ICON_WIFI_ERROR,0);
            }
            lcd_frame_update( FRAME_FLAG_ICON );
        }
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        if ( ( TermInfo.NetMode == GPRS_MODE ) || ( TermInfo.NetMode == GPRS_BAKE_MODE ) )
        {
            if( is_err )
            {
                g_fg00_info.gprs_err = 1;
                led_fg00_show_string(FG00_EVENT_MSTR,"Err   ",DISP_HOLDON_FOREVER);
            }
            else
            {
                g_fg00_info.gprs_err = 0;
                disp_state_idle();
            }
        }
        else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
        {
            if( is_err )
            {
                g_fg00_info.wifi_err = 1;
                led_fg00_show_string(FG00_EVENT_MSTR,"Err   ",DISP_HOLDON_FOREVER);
            }
            else
            {
                g_fg00_info.wifi_err = 0;
                disp_state_idle();
            }
        }
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        if( is_err )
        {
            release_lcd_queue(CONNECT_NET_FAIL, 0, NULL);
        }
        else
        {
            release_lcd_queue(CONNECT_NET_SUCCES, 0, NULL);
        }
    }
#endif
}


// disp param config state
void disp_set_param_config_error(uint8_t is_err)
{
    LOG_DBG("%s: is_err=%d\n",__func__,is_err);

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        if ( ( TermInfo.NetMode == GPRS_MODE ) || ( TermInfo.NetMode == GPRS_BAKE_MODE ) )
        {
            if( is_err )
            {
                led_fg00_show_string(FG00_EVENT_MSTR,"Err PA",DISP_HOLDON_FOREVER);
            }
            else
            {
                //g_fg00_info.gprs_err = 0;
                disp_state_idle();
            }
        }
        else if( (TermInfo.NetMode == WIFI_MODE) || (TermInfo.NetMode == WIFI_BAKE_MODE) )
        {
            if( is_err )
            {
                //g_fg00_info.wifi_err = 1;
                led_fg00_show_string(FG00_EVENT_MSTR,"Err PA",DISP_HOLDON_FOREVER);
            }
            else
            {
                //g_fg00_info.wifi_err = 0;
                disp_state_idle();
            }
        }
    }
#endif

}

// disp gprs connect state
void disp_set_charge(uint8_t is_charge)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        if ( is_charge )
        {
            // set charging flag
            lcd_set_icon( ICON_BAT_CHARGE, 1 );
        }
        else
        {
            // clear charge flag
            lcd_set_icon( ICON_BAT_CHARGE, 0 );
        }
        lcd_frame_update( FRAME_FLAG_ICON );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        if ( is_charge )
        {
            g_fg00_info.bat_charge = 1;
        }
        else
        {
            g_fg00_info.bat_charge = 0;
        }
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif
}

// disp money
void disp_set_paymsg(int64_t money,uint32_t ts,uint32_t flags)
{
    LOG_DBG("%s: money=%lld,ts=%ld\n",__func__,money,ts);
    char str_tm[24] = { 0, };
    uint32_t timestamp = ts+8*3600;
    //struct tm *tm_ptr = ql_localtime( &timestamp );
    struct tm *tm_ptr = gmtime( &timestamp );
    snprintf( str_tm, sizeof( str_tm ), "%02d%02d%02d", tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec );

#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {

    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {

        if( flags & TTS_FLAG_RECORD )
        {
            // ��¼�طţ���ʾ10s
            //g_fg00_info.yuan = 1;
            if( ts )
            {
                disp_set_tips(str_tm,10*1000);
            }
            led_fg00_show_digit(FG00_EVENT_PAYSTR,money,10*1000);
        }
        else if( flags & TTS_FLAG_CANCEL )
        {
            // ȡ��֧������ʾ-FA-
            if( ts )
            {
                disp_set_tips(str_tm,DISP_HOLDON_MS + 500);
            }
            disp_set_led_msg(" -FA- ",DISP_HOLDON_MS + 500);
        }
        else //if( flags & TTS_FLAG_PAYMSG )
        {
            //g_fg00_info.yuan = 1;
            if( ts )
            {
                disp_set_tips(str_tm,DISP_HOLDON_MS + 500);
            }
            led_fg00_show_digit(FG00_EVENT_PAYSTR,money,DISP_HOLDON_MS + 500);
        }
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(ONCE, money, NULL);
    }
#endif
}

// disp msg on main area
void disp_set_msg(char *msg,uint32_t timeout_ms)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        ui_send_msg_event( msg, timeout_ms);
    }
#endif

#if DEV_F240320_SUPPORT || DEV_F480320_SUPPORT
    if( TermInfo.disp.f240320 )
    {
        ui_send_msg_event( msg, timeout_ms);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        led_fg00_show_string(FG00_EVENT_MSTR,msg,timeout_ms);
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
    }
#endif
}

// disp tips on slave area
void disp_set_tips(char *msg,uint32_t timeout_ms)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        //ui_send_sys_tips_event( msg, timeout_ms );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
    {
        led_fg00_show_string(FG00_EVENT_SSTR,msg,timeout_ms);
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
    }
#endif
}

// set custom msg
void disp_set_custom_msg(void *data,int datalen)
{
#if DEV_F240320_SUPPORT || DEV_F480320_SUPPORT
    ui_send_event(UI_EVENT_CUSTOM_MSG,datalen,(uint8_t *)data);
#endif
}

// set msg show on screen,only for led-type device
void disp_set_led_msg(char *msg,uint32_t timeout_ms)
{
    LOG_DBG("%s: set msg %s,timeout=%ld\n",__func__,msg,timeout_ms);
#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        led_fg00_show_string(FG00_EVENT_MSTR,msg,timeout_ms);
    }
#endif

#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
    }
#endif
}

// disp QRCODE
void disp_set_qrcode(char *qr_content,int content_len)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        ui_send_event(UI_EVENT_QRCODE,content_len,qr_content);
    }
#elif DEV_F240320_SUPPORT || DEV_F480320_SUPPORT
    if( TermInfo.disp.f240320 )
    {
        ui_send_event(UI_EVENT_QRCODE,content_len,qr_content);
    }
#endif

}

// disp update request
void disp_update_request(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        ui_send_event( UI_EVENT_UPDATE, 0, NULL );
    }
#elif DEV_F240320_SUPPORT
    if ( TermInfo.disp.f240320 )
    {
        lcd_f240320_frame_update();
    }
#elif DEV_F480320_SUPPORT
    if ( TermInfo.disp.f240320 )
    {
        lcd_f480320_frame_update();
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        //led_fg00_request_update();
        fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
    }
#endif
}

// disp on/off request
void disp_onoff_request(uint8_t on,uint32_t holdon_ms)
{
    LOG_DBG("%s: set on to %d,%ldms\n",__func__,on,holdon_ms);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        ui_send_event( UI_EVENT_UPDATE, 0, NULL );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        LOG_DBG("%s: chage status %d\n",__func__,TermInfo.Charge);
        if( TermInfo.Charge )
        {
            // hold on when charging
            holdon_ms = DISP_HOLDON_FOREVER;
        }
        if( on )
        {
            led_fg00_wakeup(holdon_ms);
        }
        else
        {
            // off
            led_fg00_off();
        }
    }
#endif
}

// disp enable sleep
void disp_sleep_enable(uint8_t enable)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        led_fg00_sleep_enable(enable);
    }
#endif
}

// disp flash
void disp_show_flash(uint8_t mode, uint32_t repeat,uint32_t period_ms)
{
    LOG_DBG("%s: repeat=%ld, period=%ld\n",__func__,repeat,period_ms);

#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        led_fg00_flash(mode, repeat,period_ms);
    }
#endif
}

// disp record index
void disp_record_index(int index,uint32_t timeout_ms)
{
    LOG_DBG("%s: index=%d,ts=%ld\n",__func__,index,timeout_ms);
#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        if( index < 0 )
        {
            led_fg00_show_emsg(LED_FG00_BLANK_STR,index,timeout_ms);
        }
        else
        {
            led_fg00_show_emsg(NULL,index,timeout_ms);
        }
    }
#endif
}


// disp enable clock show
void disp_clock_enable(uint8_t enable)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        g_fg00_info.status.en_clock = enable;
        if( !enable && g_fg00_info.status.m_mode == FG00_M_MODE_CLOCK)
        {
            // ֻ�д���ʱ��ģʽ������²Ÿ�����ʾ���ݣ����������Ҫ����������ʾ������
            //disp_set_msg(LED_FG00_IDLE_STR, 500);
            fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
        }
        else
        {
            fg00_send_event(FG00_EVENT_UPDATE,0,NULL);
        }
    }
#endif
}

// disp busy state
void disp_state_busy(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(CONNECT_NET, 0, NULL);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        fg00_send_event( FG00_EVENT_BUSY, 0, NULL );
    }
#endif
}

// disp idle state
void disp_state_idle(void)
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_TM1604_SUPPORT
    if( TermInfo.disp.tm1604 )
    {
        release_lcd_queue(CONNECT_NET, 0, NULL);
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if( TermInfo.disp.fg00ahk )
     {
        if( g_fg00_info.yuan )
        {
            // 如果当前正在显示交易的信息，不要打断显示
            LOG_INFO("%s: paymsg is showing,skip idle state\n",__func__);
            return;
        }
        led_fg00_show_digit(FG00_EVENT_PAYSTR,0,DISP_HOLDON_MS + 500);
     }
#endif
}

// disp all on
void disp_factorytest_onoff(int on)
{
    LOG_DBG("%s: set on to %d\n",__func__,on);

#if DEV_FG7864_SUPPORT
    if ( TermInfo.disp.fg7864 )
    {
        if ( on )
        {
            lcd_icon_fill( );
            lcd_fill( );
        }
        else
        {
            lcd_icon_clear( );
            lcd_clear( );
        }

        lcd_frame_update( FRAME_FLAG_FLUSH );
    }
#endif

#if DEV_FG00AHK_SUPPORT
    if ( TermInfo.disp.fg00ahk )
    {
        if ( on )
        {
            led_fg00_fill( );
        }
        else
        {
            led_fg00_clear( );
        }

        fg00_send_event( FG00_EVENT_FLUSH, 0, NULL );
    }
#endif

#if DEV_TM1604_SUPPORT
    if ( TermInfo.disp.tm1604 )
    {
        if ( on )
        {
            tm_show_string( PWR_ON_SHOW );
        }
        else
        {
            tm_show_string( "      " );
        }

    }
#endif

}

void disp_set_img()
{
    LOG_DBG("%s: call\n",__func__);
#if DEV_FG7864_SUPPORT
    if( TermInfo.disp.fg7864 )
    {
        ui_send_event(UI_EVENT_QRCODE,content_len,qr_content);
    }
#elif DEV_F240320_SUPPORT || DEV_F480320_SUPPORT
    if( TermInfo.disp.f240320 )
    {
        ui_send_event(UI_EVENT_CUSTOM_IMG,0,NULL);
    }
#endif

}

