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
#ifndef __AUDIO_PLAYER_H_
#define __AUDIO_PLAYER_H_

#include "gpio_button_task.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
	CMD_PLAYER_NEXT,
	CMD_PLAYER_VOLUME_DOWN,
	CMD_PLAYER_VOLUME_UP,
	CMD_PLAYER_VOLUME_ROLL,
	CMD_PLAYER_SEL_CHANL,
	CMD_PLAYER_AP_MODE,
	CMD_PLAYER_EXIT_AP,
	CMD_PLAYER_REFACTORY,
	CMD_PLAYER_POWEROFF,
	CMD_PLAYER_TRANS_RECORD,
	CMD_PLAYER_MODE_FUNC,
	CMD_PLAYER_RECORD,
	CMD_PLAYER_SWITCH_QUERY_MODE,
//	CMD_PLAYER_LIGHT_UP,
//	CMD_PLAYER_LIGHT_DOWN,
	CMD_PLAYER_LIGHT_ON,
	CMD_PLAYER_LIGHT_OFF,
//	CMD_PLAYER_RECORDER_START,
//	CMD_PLAYER_RECORDER_PLAY,
	CMD_PLAYER_NULL,
}PLAYER_CMD;
#define BUTTON_QUEUE_TIMEOUT        5000


QlOSStatus button_task_init();
QlOSStatus button_task_deinit();
void player_set_gpio_button_cmd(GPIO_Button_Cmd_Info *cmd);
//void player_set_ad_button_cmd(AD_Button_Cmd_Info *cmd);
void button_queue_task(void);
void set_OTA_sta(int set);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif//__AUDIO_PLAYER_H_
