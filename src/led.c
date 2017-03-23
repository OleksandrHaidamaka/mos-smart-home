/*
 @autor:       Alexandr Haidamaka
 @file:        led.c
 @description: Led driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

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

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int gl_led_pwm = 100;

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
}

//------------------------------------------------------------------------------
void led_init()
{
	led_pwm(gl_led_pwm);
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
				led_mode_update:
				led_pwm(0);
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
