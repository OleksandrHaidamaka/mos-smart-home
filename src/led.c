/*
 @autor:       Alexandr Haidamaka
 @file:        led.c
 @description: Led driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/platforms/esp8266/esp_missing_includes.h"
#include "fw/src/mgos_gpio.h"
#include "fw/platforms/esp8266/src/esp_gpio.h"
#include "fw/platforms/esp8266/src/esp_periph.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/

/*******************************************************************************
 *** TYPEDEFS
 ******************************************************************************/
struct blink_mode_t
{
	int time_short;
	int time_long;
	int times;
	bool repeat;
};

#define PWM_BASE_RATE_US 50
/* The following constants are used to get the base 10 KHz freq (100 uS period)
 * used to drive PWM. */
#define TMR_PRESCALER_16 4 /* 16x prescaler */
/* At 80 MHZ timer clock source is 26 MHz and each unit of this adds 0.2 uS. */
#define TMR_RELOAD_VALUE_80 (250 - 8)
/* At 160, the frequency is the same but the constant fraction that accounts for
 * interrupt handling code running before reload needs to be adjusted. */
#define TMR_RELOAD_VALUE_160 (250 - 4)

#define FRC1_ENABLE_TIMER BIT7
#define TM_INT_EDGE 0

//IRAM NOINSTR void tim_callback(void *arg)
//{
//	(void) arg;
//}
//
//void timer()
//{
//	ETS_FRC_TIMER1_INTR_ATTACH(tim_callback, NULL);
//	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
//	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, TMR_RELOAD_VALUE_80);
//	RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
//	TMR_PRESCALER_16 | FRC1_ENABLE_TIMER | TM_INT_EDGE);
//	TM1_EDGE_INT_ENABLE();
//	ETS_FRC1_INTR_ENABLE();
//}

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int gl_led_pwm = 100; // range (0 - 100)  0 - off 100 - maximum

struct blink_mode_t main_blink_mode[] =
{
//       time short, time long, number of short blinks, repeat flag
		{ 250 / SYS_TICK, 500 / SYS_TICK, 1, true }, // BL_WIFI_DISCONNECTED
		{ 250 / SYS_TICK, 500 / SYS_TICK, 2, true }, // BL_WIFI_IP_ACQUIRED
		{ 250 / SYS_TICK, 500 / SYS_TICK, 3, true }, // BL_MQTT_CONNECTED
		{ 50 / SYS_TICK, 100 / SYS_TICK, 1, false }, // BL_MQTT_SUB_MSG_ERR
		{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }, // BL_MQTT_SUB_MSG
		{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }  // BL_MQTT_PUB_MSG
};

enum bl_mode bl_mode_current = BL_WIFI_DISCONNECTED;
enum bl_mode bl_mode_new = BL_WIFI_DISCONNECTED;

//------------------------------------------------------------------------------
void led_pwm(int dim)  // range (0 - 100)  0 - off 100 - maximum
{
	dim *= 100;
	dim = 10000 - dim;
	mgos_pwm_set(LED_PIN, 10000, dim);
//	timer();
}

//------------------------------------------------------------------------------
void led_init()
{
	led_pwm(gl_led_pwm);
	blink_mode(BL_WIFI_DISCONNECTED);
}

void blink_mode(enum bl_mode mode)
{
	bl_mode_new = mode;

	if (main_blink_mode[mode].repeat == true)
		bl_mode_current = mode;
}

//------------------------------------------------------------------------------
void led_driver()
{
	static enum bl_mode mode = BL_WIFI_DISCONNECTED;
	static int time = 0;
	static int state = 0;
	static int count = 0;
	static bool go_to_bl_mode_current = false;

	if (time != 0)
		time--;

	if (mode != bl_mode_new)
	{
		state = 0;
		goto led_mode_update;
	}

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count >= main_blink_mode[mode].times)
			{
				led_mode_update: led_pwm(0);
				count = 0;
				time = main_blink_mode[mode].time_long;

				mode = bl_mode_new;

				if (go_to_bl_mode_current == true)
				{
					go_to_bl_mode_current = false;
					mode = bl_mode_new = bl_mode_current;
				}
				if (main_blink_mode[mode].repeat == false)
					go_to_bl_mode_current = true;

				break;
			}
			led_pwm(gl_led_pwm);
			time = main_blink_mode[mode].time_short;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_pwm(0);
			time = main_blink_mode[mode].time_short;
			state--;
		}
		break;
	}
}
