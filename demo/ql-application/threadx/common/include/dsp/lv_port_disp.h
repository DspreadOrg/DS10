/**
 * @file lv_port_disp_templ.h
 *
 */

 /*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void (*disp_flush)(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t *buff);
	void * buff1;
	void * buff2;
	int bufsize;
	int width;
	int heigh;
}LcdColorDrv;

extern const LcdColorDrv * pLcdColorDrv;

/*********************
 *      INCLUDES
 *********************/
//#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define COMBINE_LV_PORT_TASK         1

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

void lv_port_disp_init(void);

void disp_console_show();

void lv_set_inverse(int inv);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
