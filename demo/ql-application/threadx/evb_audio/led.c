#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_application.h"

#include "prj_common.h"
#include "public_api_interface.h"
#include "led.h"
#include "gpio.h"

#define LOG_DBG(fmt, arg...) //printf("[LED DBG]"fmt, ##arg)
#define LOG_INFO(fmt, arg...) printf("[LED INFO]"fmt, ##arg)

struct LedMode{
	 char Step;
	 LedStatSeq LedStat[5];
};
struct LedMode LedStatSetNext;

char LedReadyFlag=0;
static void Led_task(u32 argv);
static void LedModeSet(int LedSel,LedStatSet Mode);

ql_sem_t LedModeChang = NULL;
ql_task_t Led_task_ctrl_thread = NULL;


#define LED_TASK_CTRL_THREAD_STACK_SIZE	(1024)

void ChargeStatLedSet(int OnOff)
{
}

void gpio_led_task_init(void)
{
	QlOSStatus ret = 0;
	
	ret = ql_rtos_semaphore_create(&LedModeChang, 0);
	if(ret)
	{
		LOG_INFO("========== led semaphore create error\r\n");
		return ;
	}
//	LedModeSet(2,LedStartMode)
	LedModeSet(GREEN_LED_SET|BLUE_LED_SET|RED_LED_SET,AlwaysOn);
//	LedModeSet(RED_LED_SET,AlwaysOn);	
	ret = ql_rtos_task_create(&Led_task_ctrl_thread, LED_TASK_CTRL_THREAD_STACK_SIZE, 100, "Led_task", Led_task, NULL);
	if(ret)
	{
		LOG_INFO("========== led thread create error\r\n");
		return ;
	}
}

void gpio_led_task_deinit(void)
{
    if (Led_task_ctrl_thread != NULL)
    {
        Led_task_ctrl_thread = NULL;
        ql_rtos_task_delete(NULL);
    }
}

const int SlowTime[2]={600,800};
const int FastTime[2]={100,200};
const int BlinkTime=300;

static void Led_task(u32 argv)
{
	QlOSStatus ret = 0;
	unsigned int time;
	int step=0;
	struct LedMode LedModeCur;//=LedStatSetNext;
	LedStatSeq * ledOut;
	
	LedReadyFlag=1;
	while(1)
	{
		LedModeCur=LedStatSetNext;
		while(1)
		{
			if (step>=LedModeCur.Step) step=0;
			ledOut=&LedModeCur.LedStat[step];
            
            setRedLedOnOff((ledOut->LedSet&RED_LED_SET)?1:0);
            setBlueLedOnOff((ledOut->LedSet&BLUE_LED_SET)?1:0);
            setGreenLedOnOff((ledOut->LedSet&GREEN_LED_SET)?1:0);
            
            if (ledOut->SetTimeMs==-1)
				time = 0xFFFFFFFF;
			else
				time = ledOut->SetTimeMs;

			ret = ql_rtos_semaphore_wait(LedModeChang, time);
			if (ret == 0) 
			{
				break;
			}
			step++;
		}
	}
}

static void LedModeSet(int LedSel,LedStatSet Mode)
{
	LedStatSeq * set;
	int ii;

	set=LedStatSetNext.LedStat;

    if (Mode == Breath)
    {
        Mode = FlashSlow;
    }

	setBreathLedMode(GPIO_TYPE);
	if(Mode==Breath)
	{
		set->LedSet=0;
		set->SetTimeMs=-1;
		LedStatSetNext.Step=1;
		setBreathLedMode(PWM_TYPE);
	}
	
	switch(Mode)
	{
		case Blink:
			for(ii=0;ii<8;ii++)
			{
				if (LedSel&(1<<ii))
				{
					set->LedSet=1<<ii;
					set->SetTimeMs=BlinkTime;
					set++;
				}
			}
			for(ii=0;ii<8;ii++)
				if (&LedStatSetNext.LedStat[ii]==set) break;
			if (ii==1)
			{
				set->LedSet=0;
				set->SetTimeMs=BlinkTime;
				ii++;
			}
			LedStatSetNext.Step=ii;
			break;
		case FlashFast:
			set->LedSet=LedSel;
			set->SetTimeMs=FastTime[0];
			set++;
			set->LedSet=0;
			set->SetTimeMs=FastTime[1];
			LedStatSetNext.Step=2;				
			break;
		case FlashSlow:
			set->LedSet=LedSel;
			set->SetTimeMs=SlowTime[0];
			set++;
			set->LedSet=0;
			set->SetTimeMs=SlowTime[1];
			LedStatSetNext.Step=2;
			break;
		case AlwaysOn:
			set->LedSet=LedSel;
			set->SetTimeMs=-1;
			LedStatSetNext.Step=1;
			break;
		case AlwaysOff:
			set->LedSet=0;
			set->SetTimeMs=-1;
			LedStatSetNext.Step=1;
			break;
		case Breath:
			setBreathLedTime(-1);//��λms 
			break;
	}

	ql_rtos_semaphore_release(LedModeChang);
}

/******************************************************************************
  Function: TermLedShow
  Description: Terminal les show status
  INPUT: mode
  OUTPUT: 
  Return: 
 ******************************************************************************/
void TermLedShow(LedModeDef mode)
{
    int red_led_support = 0;
    dev_config_t *pdevconf = get_device_config();
    if( pdevconf->pins->led_red.gpio_num < (GPIO_PIN_NO_MAX+1) )
    {
        red_led_support = 1;
    }

    usb_log_printf("%s: set mode %d\n",__func__,mode);
	switch(mode)
	{
        case TERM_ABNORMAL:
        case TERM_NET_START:
        case TERM_NO_SIM:
            if ( red_led_support )
            {
                LedModeSet( RED_LED_SET, AlwaysOn );
            }
            else
            {
                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            break;

        case TERM_ABNORMAL_EXT:
            if ( red_led_support )
            {
                LedModeSet( RED_LED_SET | GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            else
            {
                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            break;
        
		case TERM_INIT_START:		LedModeSet(GREEN_LED_SET,AlwaysOn);			               		break;
		case TERM_INIT_END:			LedModeSet(BLUE_LED_SET|RED_LED_SET|GREEN_LED_SET,Blink);   	break;
		case TERM_OTA_START:	    LedModeSet(RED_LED_SET|GREEN_LED_SET|BLUE_LED_SET,Blink);     	break;
        
		case TERM_OTA_FAIL:
		    if ( red_led_support )
            {
                LedModeSet( RED_LED_SET, Blink );
            }
            else
            {
                LedModeSet( GREEN_LED_SET, Blink );
            }
            break;
            
		case TERM_OTA_OK:		    LedModeSet(GREEN_LED_SET,AlwaysOn);	               				break;

		case TERM_NET_DIS:
		    if ( red_led_support )
            {
                LedModeSet( BLUE_LED_SET | RED_LED_SET, Blink );
            }
            else
            {
                LedModeSet( BLUE_LED_SET | GREEN_LED_SET, Blink );
            }
            break;
        
		case TERM_NET_CON:		    LedModeSet(BLUE_LED_SET,FlashSlow);	               				break;
        case TERM_NET_ABNORMAL:		LedModeSet(BLUE_LED_SET,AlwaysOn);	               				break;
        case TERM_LTE_NET_ABNORMAL:	LedModeSet(BLUE_LED_SET, Breath);	               				break;

        case TERM_PARAM_ERR:
		case TERM_REG_FAIL:
		    if ( red_led_support )
            {
                LedModeSet( RED_LED_SET, AlwaysOn );
            }
            else
            {
                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            break;
            
		case TERM_CHARGE_START:
		    if ( red_led_support )
            {
                LedModeSet( RED_LED_SET, FlashFast );
            }
            else
            {
                LedModeSet( RED_LED_SET | GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            break;

		case TERM_CHARGE_FULL:	    LedModeSet(GREEN_LED_SET, AlwaysOn);               		        break;
        
		case TERM_LOWBAT:
		    if ( red_led_support )
            {
                LedModeSet( GREEN_LED_SET | RED_LED_SET, Blink );
            }
            else
            {
                LedModeSet( GREEN_LED_SET | BLUE_LED_SET, Blink );
            }
            break;
            
		case TERM_POWEROFF:		    LedModeSet(BLUE_LED_SET|RED_LED_SET|GREEN_LED_SET, AlwaysOff);	break;
		default:			        break;
	}
}

//application_init(quec_led_test, "quec_led_test", 2, 0);

