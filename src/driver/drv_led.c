/*
 @autor:       Alexandr Haidamaka
 @file:        drv_led.c
 @description: driver led functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define LED_PWM_PERIOD  (10000) // [us]

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int gl_drv_led_pwm = 100; // range (0 - 100)  0 - off 100 - maximum
bool gl_drv_led_mqtt = true;

drv_blink_mode_t drv_blink_mode[] =
{
//       time short, time long, number of short blinks, repeat flag
		{ 250 / SYS_TICK, 500 / SYS_TICK, 1, true }, // BL_WIFI_DISCONNECTED
		{ 250 / SYS_TICK, 500 / SYS_TICK, 2, true }, // BL_WIFI_IP_ACQUIRED
		{ 250 / SYS_TICK, 500 / SYS_TICK, 3, true }, // BL_MQTT_CONNECTED
		{ 50 / SYS_TICK, 100 / SYS_TICK, 1, false }, // BL_MQTT_SUB_MSG_ERR
		{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }, // BL_MQTT_SUB_MSG
		{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }  // BL_MQTT_PUB_MSG
};

enum drv_led_blink_mode_t drv_led_blink_mode_current = BL_WIFI_DISCONNECTED;
enum drv_led_blink_mode_t drv_led_blink_mode_new = BL_WIFI_DISCONNECTED;

inline static int dim_to_duty(int dim)
{
	dim *= (LED_PWM_PERIOD / 100);
	return (LED_PWM_PERIOD - dim);
}

//------------------------------------------------------------------------------
void drv_led_pwm(int dim)  // range (0 - 100)  0 - off 100 - maximum
{
	pwm_set(LED_PIN, LED_PWM_PERIOD, dim_to_duty(dim));
}

//------------------------------------------------------------------------------
void drv_led_init()
{
	drv_led_pwm(gl_drv_led_pwm);
	drv_led_blink_mode(BL_WIFI_DISCONNECTED);
}

void drv_led_blink_mode(enum drv_led_blink_mode_t mode)
{
	drv_led_blink_mode_new = mode;

	if (drv_blink_mode[mode].repeat == true)
		drv_led_blink_mode_current = mode;
}

//------------------------------------------------------------------------------
void drv_led_handler()
{
	static enum drv_led_blink_mode_t mode = BL_WIFI_DISCONNECTED;
	static int time = 0;
	static int state = 0;
	static int count = 0;
	static bool go_to_bl_mode_current = false;

	if (time != 0)
		time--;

	if (mode != drv_led_blink_mode_new)
	{
		state = 0;
		goto led_mode_update;
	}

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count >= drv_blink_mode[mode].times)
			{
				led_mode_update: drv_led_pwm(0);
				count = 0;
				time = drv_blink_mode[mode].time_long;

				mode = drv_led_blink_mode_new;

				if (go_to_bl_mode_current == true)
				{
					go_to_bl_mode_current = false;
					mode = drv_led_blink_mode_new = drv_led_blink_mode_current;
				}
				if (drv_blink_mode[mode].repeat == false)
					go_to_bl_mode_current = true;

				break;
			}
			drv_led_pwm(gl_drv_led_pwm);
			time = drv_blink_mode[mode].time_short;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			drv_led_pwm(0);
			time = drv_blink_mode[mode].time_short;
			state--;
		}
		break;
	}
}
