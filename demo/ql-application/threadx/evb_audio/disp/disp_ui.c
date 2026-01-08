/**
  ******************************************************************************
  * @file    disp_ui.c
  * @author  Xu
  * @version V1.0.0
  * @date    2021/11/03
  * @brief   ui func file for C code
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
#include "ql_type.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_spi.h"
#include "prj_common.h"
#include "systemparam.h"
#include "public_api_interface.h"
#include "terminfodef.h"
#include "lv_port_disp.h"
#include "animate.h"
#include "disp_ui.h"
#include "disp_port.h"
#include "disp_strdef.h"

#define LOG_INFO(fmt, arg...) usb_log_printf("[INFO DISP]"fmt, ##arg)

/* Private typedef -----------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define QR_CONTENT_DEFAULT      "Welcome to use the cloud speaker"

//#define WELCOME_MSG             "\xe6\xac\xa2\xe8\xbf\x8e\xe4\xbd\xbf\xe7\x94\xa8"
//#define YUNYINXIANG             "\xe4\xba\x91\xe9\x9f\xb3\xe7\xae\xb1"
//#define POWER_ON_MSG            "\xe6\xac\xa2\xe8\xbf\x8e\xe4\xbd\xbf\xe7\x94\xa8\n  \xe4\xba\x91\xe9\x9f\xb3\xe7\xae\xb1"
//#define PAY_REQUEST_MSG         "\xe8\xaf\xb7\xe6\x89\xab\xe7\xa0\x81\xe6\x94\xaf\xe4\xbb\x98"
//#define PAY_OK_MSG              "\xe6\x94\xaf\xe4\xbb\x98\xe6\x88\x90\xe5\x8a\x9f"
//#define PAY_THRANKS_MSG         "\xe8\xb0\xa2\xe8\xb0\xa2\xe6\x83\xa0\xe9\xa1\xbe"
//#define REFUND_REQUEST_MSG      "\xe8\xaf\xb7\xe6\x89\xab\xe7\xa0\x81\xe9\x80\x80\xe6\xac\xbe"
//#define YUAN                    "\xe5\x85\x83"

#if DEV_FG7864_SUPPORT
#define SCREEN_WIDTH                     64
#define SCREEN_HEIGHT                    78
#define UI_FONT                 font_msyh_14
#elif DEV_F240320_SUPPORT
#define SCREEN_WIDTH                     240
#define SCREEN_HEIGHT                    320
#define UI_FONT                 font_msyh_30
#elif DEV_F480320_SUPPORT 
#define SCREEN_WIDTH                     480
#define SCREEN_HEIGHT                    320
#define UI_FONT                 font_msyh_30
#else
#define SCREEN_WIDTH                     64
#define SCREEN_HEIGHT                    78
#define UI_FONT                 font_msyh_14
#endif

#define LOGO_FONT               font_hwhp_40
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
lv_obj_t *g_bg = NULL;
lv_obj_t *g_lv_welcom_msg = NULL;
lv_obj_t *g_lv_qr = NULL;
lv_obj_t *g_lv_img_qsmzf = NULL;
//lv_obj_t *g_lv_img_signal = NULL;
//lv_obj_t *g_lv_img_main = NULL;
lv_obj_t *g_top_bg_full = NULL;
lv_obj_t *g_lv_sys_tips = NULL;
static uint32_t timer_lv_top_tips = 0;

lv_obj_t *g_lv_msg = NULL;
static uint32_t timer_lv_msg = 0;

ql_task_t lv_ui_show_ctrl = NULL;
ql_queue_t lv_ui_event_queue = NULL;

/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
static void lv_ui_handle_task(void *pvParameters);

// ui send event
int ui_send_event(uint8_t event_type,uint32_t datalen,uint8_t *data )
{
    ui_event_t ui_event;

    if( NULL == lv_ui_event_queue )
    {
        return -1;
    }

    memset(&ui_event,0,sizeof(ui_event_t));

    if( datalen && data )
    {
        ui_event.data = calloc(datalen+1,1);
        if( ui_event.data == NULL )
        {
            LOG_INFO("%s: not enough mem\n",__func__);
            return -1;
        }
        memcpy(ui_event.data,data,datalen);
        ui_event.datalen = datalen;
    }

    ui_event.type = event_type;

    uint32_t msg_num;
    ql_rtos_queue_get_cnt( lv_ui_event_queue, &msg_num );
    LOG_INFO( "%s: event num in queue: %ld\n", __func__, msg_num );

    if (0 != ql_rtos_queue_release(lv_ui_event_queue, sizeof(ui_event_t), (u8*)&ui_event, QL_NO_WAIT))
    {
        LOG_INFO("%s: send event fail\n",__func__);
        if( ui_event.data )
        {
            free(ui_event.data);
        }
        return -1;
    }

    return 0;
}

// send sys tips event,this msg will be show on top
int ui_send_sys_tips_event(char *msg,uint32_t timeout_ms)
{
    ui_tips_data_t tips;
    memset(&tips,0,sizeof(ui_tips_data_t));
    tips.timeout_ms = timeout_ms;
    strncpy(tips.text,msg,sizeof(tips.text) - 1);

    LOG_INFO("%s: send tips %s,time %ldms\n",__func__,tips.text,timeout_ms);

    return ui_send_event(UI_EVNET_SYS_TIPS,sizeof(ui_tips_data_t),(uint8_t *)&tips);
}

// send normal msg event
int ui_send_msg_event(char *msg,uint32_t timeout_ms)
{
    ui_tips_data_t tips;
    memset(&tips,0,sizeof(ui_tips_data_t));
    tips.timeout_ms = timeout_ms;
    if ( msg )
    {
        strncpy( tips.text, msg, sizeof( tips.text ) - 1 );
    }

    LOG_INFO("%s: send tips %s,time %ldms\n",__func__,tips.text,timeout_ms);

    return ui_send_event(UI_EVNET_MSG,sizeof(ui_tips_data_t),(uint8_t *)&tips);
}

// show sys tips
static int set_sys_tips(char *msg,uint32_t timeout_ms)
{
    if( !g_lv_sys_tips )
    {
        return 0;
    }

    lv_label_set_text( g_lv_sys_tips, msg );
    lv_obj_clear_flag( g_top_bg_full, LV_OBJ_FLAG_HIDDEN );
    timer_lv_top_tips = ql_rtos_get_systicks_to_ms() + timeout_ms;

    return 0;
}

// show normal message
static int set_show_msg(char *msg,uint32_t timeout_ms)
{
    if( !g_lv_msg )
    {
        return 0;
    }

    lv_label_set_text( g_lv_msg, msg );
    lv_obj_clear_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
    timer_lv_msg = ql_rtos_get_systicks_to_ms( ) + timeout_ms;

    return 0;
}

// show custom message
static int set_show_custom_msg(ui_custom_msg_t *pmsg)
{
    if( !g_lv_msg )
    {
        return 0;
    }

    switch( pmsg->type )
    {
        case UI_MSG_PAY_REQUEST:
        {
            lv_obj_add_flag( g_lv_welcom_msg, LV_OBJ_FLAG_HIDDEN );
            lv_qrcode_update( g_lv_qr, QR_CONTENT_DEFAULT, strlen(QR_CONTENT_DEFAULT) );
            lv_obj_clear_flag( g_lv_qr, LV_OBJ_FLAG_HIDDEN );

            int64_t money = pmsg->data.pay.money;
            if( money < 0 )
            {
                money = -money;
            }
            char f_str[100] = {0,};
            if(money % 100 )
            {
                snprintf( f_str, sizeof( f_str ), "#00000f "PAY_REQUEST_MSG"#\n#bb0304 %lld.%lld%lld"YUAN"#",
                        money / 100, ( money % 100 ) / 10, money % 10 );
            }
            else
            {
                snprintf(f_str,sizeof(f_str),"#00000f "PAY_REQUEST_MSG"#\n#bb0304 %lld"YUAN"#",money/100);
            }
            lv_label_set_text( g_lv_msg, f_str);
            lv_obj_clear_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
            timer_lv_msg = ql_rtos_get_systicks_to_ms() + pmsg->timeout_ms;
        }
            break;

        case UI_MSG_REFUND_REQUEST:
        {
            lv_obj_add_flag( g_lv_welcom_msg, LV_OBJ_FLAG_HIDDEN );
            lv_qrcode_update( g_lv_qr, QR_CONTENT_DEFAULT, strlen(QR_CONTENT_DEFAULT) );
            lv_obj_clear_flag( g_lv_qr, LV_OBJ_FLAG_HIDDEN );

            int64_t money = pmsg->data.pay.money;
            if( money < 0 )
            {
                money = -money;
            }
            char f_str[100] = {0,};
            if(money % 100 )
            {
                snprintf( f_str, sizeof( f_str ), "#00000f "REFUND_REQUEST_MSG"#\n#008000 %lld.%lld%lld"YUAN"#",
                        money / 100, ( money % 100 ) / 10, money % 10 );
            }
            else
            {
                snprintf(f_str,sizeof(f_str),"#00000f "REFUND_REQUEST_MSG"#\n#008000 %lld"YUAN"#",money/100);
            }
            lv_label_set_text( g_lv_msg, f_str);
            lv_obj_clear_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
            timer_lv_msg = ql_rtos_get_systicks_to_ms() + pmsg->timeout_ms;
        }
            break;

        case UI_MSG_PAY_OK:
            lv_obj_add_flag( g_lv_welcom_msg, LV_OBJ_FLAG_HIDDEN );

            lv_label_set_text( g_lv_msg, "#F8F8FF "PAY_OK_MSG"!#" );
            lv_obj_clear_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
            timer_lv_msg = ql_rtos_get_systicks_to_ms() + pmsg->timeout_ms;
            break;

        default:
            break;

    }
    return 0;
}

// ui event service
static int ui_event_service(void)
{
    int min_period_ms = 30 * 1000;
    int period_ms;
    uint32_t tick_ms = ql_rtos_get_systicks_to_ms();
    if( timer_lv_top_tips && g_lv_sys_tips )
    {
        if( tick_ms >= timer_lv_top_tips )
        {
            // hide tips
            timer_lv_top_tips = 0;
            lv_obj_add_flag( g_top_bg_full, LV_OBJ_FLAG_HIDDEN );
            LOG_INFO("%s: top tips hide\n",__func__);
        }
        else
        {
            period_ms = 1 + timer_lv_top_tips - tick_ms;
            min_period_ms = (period_ms < min_period_ms)?period_ms:min_period_ms;
        }
    }
    if( timer_lv_msg && g_lv_msg )
    {
        if( tick_ms >= timer_lv_msg )
        {
            // hide tips
            timer_lv_msg = 0;
            lv_obj_add_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
            LOG_INFO("%s: line tips hide\n",__func__);

            // return to main page
            lv_obj_add_flag( g_lv_qr, LV_OBJ_FLAG_HIDDEN );
            lv_obj_clear_flag( g_lv_welcom_msg, LV_OBJ_FLAG_HIDDEN );
        }
        else
        {
            period_ms = 1 + timer_lv_msg - tick_ms;
            min_period_ms = (period_ms < min_period_ms)?period_ms:min_period_ms;
        }
    }

    return min_period_ms;
}

// display ui init
int ui_init(void)
{
    if (ql_rtos_queue_create(&lv_ui_event_queue, sizeof(ui_event_t), 50) != 0)
    {
        LOG_INFO("%s: create event queue fail\n",__func__);
        return -1;
    }

    if (ql_rtos_task_create(&lv_ui_show_ctrl,
                            15 * 1024,
                            100,
                            "lv_ui_show",
                            lv_ui_handle_task,
                            NULL) != 0)
    {
        LOG_INFO( "%s: thread create error\n", __func__ );
    }

    ql_rtos_task_sleep_ms(100);
    return 0;
}

static void lv_ui_handle_task(void *pvParameters)
{
    printf("%s: start\n",__func__);
    ui_event_t ui_event;
    uint32_t pre_tick_ms = ql_rtos_get_systicks_to_ms();
    uint32_t wait_ms = -1;
    uint32_t min_wait_ms;
		

#if 1
    // UI init
    LV_FONT_DECLARE(UI_FONT);
    LV_FONT_DECLARE(LOGO_FONT);
#endif

    lv_obj_set_style_bg_color( lv_scr_act(), SCREEN_BG_COLOR, 0 );

    g_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_bg,SCREEN_WIDTH,SCREEN_HEIGHT);
    lv_obj_set_style_radius(g_bg,0,0);
    lv_obj_set_style_outline_width(g_bg,0,0);
    lv_obj_set_style_border_width(g_bg,0,0);
    lv_obj_set_style_bg_color(g_bg,lv_color_hex(0x1E90FF),0);

    g_lv_welcom_msg = lv_label_create( g_bg );
#if (DEV_F240320_SUPPORT) || DEV_F480320_SUPPORT
    lv_obj_set_style_text_font( g_lv_welcom_msg, &LOGO_FONT, 0 );
	lv_obj_set_style_text_color(g_lv_welcom_msg,lv_color_white(),LV_PART_MAIN);
#else
    lv_obj_set_style_text_font( g_lv_welcom_msg, &UI_FONT, 0 );
#endif
    lv_label_set_recolor(g_lv_welcom_msg, true);
    lv_obj_set_style_text_align(g_lv_welcom_msg,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_align( g_lv_welcom_msg, LV_ALIGN_CENTER, 0, 0 );
    lv_label_set_text( g_lv_welcom_msg, POWER_ON_MSG );

    /*Create a 64x64 QR code*/
    g_lv_qr = lv_qrcode_create(g_bg, QR_CODE_SIZE, SCREEN_FT_COLOR, SCREEN_BG_COLOR);
    //v_obj_set_align(g_lv_qr,LV_ALIGN_CENTER);
    if( TermInfo.disp.f240320 )
    {
        lv_obj_align( g_lv_qr, LV_ALIGN_CENTER, 0, -40 );
    }
    else
    {
        lv_obj_align( g_lv_qr, LV_ALIGN_CENTER, 0, 0 );
    }
    lv_qrcode_update(g_lv_qr, QR_CONTENT_DEFAULT, strlen(QR_CONTENT_DEFAULT));
    lv_obj_add_flag(g_lv_qr,LV_OBJ_FLAG_HIDDEN);

    // img qing shao ma zhi fu
#if 0
    g_lv_img_qsmzf = lv_img_create( g_bg );
    lv_img_set_src( g_lv_img_qsmzf, &img_qsmzf_16x64 );
    lv_obj_align( g_lv_img_qsmzf, LV_ALIGN_CENTER, 0, 40 );
    lv_obj_add_flag( g_lv_img_qsmzf, LV_OBJ_FLAG_HIDDEN );
#endif

    // g_lv_img_qsmzf = lv_img_create(g_bg);
    // lv_img_set_src(g_lv_img_qsmzf,"B:/img/sale.png");
    // lv_obj_add_flag( g_lv_img_qsmzf, LV_OBJ_FLAG_HIDDEN );

//    LV_IMG_DECLARE( img_paysuccess_192x192 );
//    g_lv_img_main = lv_img_create( g_bg );
//    lv_obj_set_size(g_lv_img_main,QR_CODE_SIZE,QR_CODE_SIZE);
//#if (DEV_F240320_SUPPORT)
//    lv_img_set_src( g_lv_img_main, &img_paysuccess_192x192 );
//    //lv_img_set_zoom(g_lv_img_main,192);
//#endif
//    lv_obj_align( g_lv_img_main, LV_ALIGN_CENTER, 0, -40 );
//    lv_obj_add_flag( g_lv_img_main, LV_OBJ_FLAG_HIDDEN );

#if 1
    g_lv_msg = lv_label_create( g_bg );
    //lv_obj_set_size(g_lv_msg,SCREEN_WIDTH,100);
    lv_obj_set_style_text_font( g_lv_msg, &UI_FONT, 0 );
    lv_label_set_recolor(g_lv_msg, true);  // enable recolor
    lv_obj_set_style_text_align(g_lv_msg,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_align( g_lv_msg, LV_ALIGN_CENTER, 0, 110 );
    lv_label_set_text(g_lv_msg, "#ff0080 color1##00ff80 color2#");  // show lable with 2 color
    lv_obj_add_flag( g_lv_msg, LV_OBJ_FLAG_HIDDEN );
#endif

#if 1
    g_top_bg_full = lv_obj_create(lv_layer_top());
    lv_obj_set_size(g_top_bg_full,SCREEN_WIDTH,SCREEN_HEIGHT);
    lv_obj_set_style_radius(g_top_bg_full,0,0);
    lv_obj_set_style_outline_width(g_top_bg_full,0,0);
    lv_obj_set_style_border_width(g_top_bg_full,0,0);
    //lv_obj_set_style_bg_color( g_top_bg_full, SCREEN_FT_COLOR, 0 );

    g_lv_sys_tips = lv_label_create( g_top_bg_full );
    lv_obj_set_style_text_font( g_lv_sys_tips, &UI_FONT, 0 );
    //lv_label_set_text( g_lv_sys_tips, POWER_ON_MSG );
    lv_obj_center(g_lv_sys_tips);
    //lv_obj_align( g_lv_sys_tips, LV_ALIGN_CENTER, 0, 0 );

    lv_obj_add_flag( g_top_bg_full, LV_OBJ_FLAG_HIDDEN );
#endif

    lv_timer_handler();
    ql_rtos_task_sleep_ms(100);
		
    while(1)
    {
        // animates
        min_wait_ms = 10 * 1000;

        // service
        wait_ms = ui_event_service();
        if( wait_ms < min_wait_ms ) min_wait_ms = wait_ms;

#if COMBINE_LV_PORT_TASK
        // handle lv ticks
        lv_tick_inc(ql_rtos_get_systicks_to_ms() - pre_tick_ms);
        pre_tick_ms = ql_rtos_get_systicks_to_ms();

        // handle lv timer event
        lv_timer_handler();
#endif

        LOG_INFO("%s: wait event timeout %ldms\n",__func__,min_wait_ms);
        if (0 == ql_rtos_queue_wait(lv_ui_event_queue, (u8*)&ui_event, sizeof(ui_event_t), min_wait_ms))
        {
            LOG_INFO("%s: recv ui event %d\n",__func__,ui_event.type);

            if( ui_event.type == UI_EVNET_SYS_TIPS )
            {
                ui_tips_data_t *ptips = (ui_tips_data_t *)ui_event.data;
                set_sys_tips(ptips->text,ptips->timeout_ms);
            }
            else if( ui_event.type == UI_EVNET_MSG )
            {
                ui_tips_data_t *ptips = (ui_tips_data_t *)ui_event.data;
                set_show_msg(ptips->text,ptips->timeout_ms);
            }
            else if( ui_event.type == UI_EVENT_QRCODE )
            {
                lv_obj_add_flag(g_lv_welcom_msg,LV_OBJ_FLAG_HIDDEN);
                lv_qrcode_update(g_lv_qr,ui_event.data,ui_event.datalen);
                lv_obj_clear_flag(g_lv_qr,LV_OBJ_FLAG_HIDDEN);
            }
            else if ( ui_event.type == UI_EVENT_CUSTOM_MSG )
            {
                ui_custom_msg_t *pmsg = (ui_custom_msg_t*) ui_event.data;
                set_show_custom_msg( pmsg );
            }

            if( ui_event.data )
            {
                free(ui_event.data );
                ui_event.data = NULL;
            }
        }

    }
}









