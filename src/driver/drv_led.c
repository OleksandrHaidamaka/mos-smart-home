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
 *** VARIABLES
 ******************************************************************************/
drv_led_t drv_led =
{
	.mode =
	{
	//      time short, time long, number of short blinks, repeat flag
			{ 250 / SYS_TICK, 500 / SYS_TICK, 1, true }, // BL_WIFI_DISCONNECTED
			{ 250 / SYS_TICK, 500 / SYS_TICK, 2, true }, // BL_WIFI_IP_ACQUIRED
			{ 250 / SYS_TICK, 500 / SYS_TICK, 3, true }, // BL_MQTT_CONNECTED
			{ 50 / SYS_TICK, 100 / SYS_TICK, 1, false }, // BL_MQTT_SUB_MSG_ERR
			{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }, // BL_MQTT_SUB_MSG
			{ 50 / SYS_TICK, 100 / SYS_TICK, 3, false }  // BL_MQTT_PUB_MSG
	}
};

//------------------------------------------------------------------------------
void drv_led_init()
{
	led_out();
	led_off_on(true);
	drv_led.handler = drv_led_handler;
	drv_led.mode_current = BL_WIFI_DISCONNECTED;
	drv_led.mode_new = BL_WIFI_DISCONNECTED;
	drv_led_blink_mode(BL_WIFI_DISCONNECTED);
}

//------------------------------------------------------------------------------
void drv_led_blink_mode(enum drv_led_blink_mode_t mode)
{
	drv_led.mode_new = mode;

	if (drv_led.mode[mode].repeat == true)
		drv_led.mode_current = mode;
}

//------------------------------------------------------------------------------
void drv_led_handler(void)
{
	static enum drv_led_blink_mode_t mode = BL_WIFI_DISCONNECTED;
	static int time = 0;
	static int state = 0;
	static int count = 0;
	static bool go_to_bl_mode_current = false;

	if (time != 0)
		time--;

	if (mode != drv_led.mode_new)
	{
		state = 0;
		goto led_mode_update;
	}

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count >= drv_led.mode[mode].times)
			{
				led_mode_update: led_off_on(false);
				count = 0;
				time = drv_led.mode[mode].time_long;

				mode = drv_led.mode_new;

				if (go_to_bl_mode_current == true)
				{
					go_to_bl_mode_current = false;
					mode = drv_led.mode_new = drv_led.mode_current;
				}
				if (drv_led.mode[mode].repeat == false)
					go_to_bl_mode_current = true;

				break;
			}
			led_off_on(true);
			time = drv_led.mode[mode].time_short;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_off_on(false);
			time = drv_led.mode[mode].time_short;
			state--;
		}
		break;
	}
}

//------------------------------------------------------------------------------
void drv_LED_handler(void)
{
	if (drv_led.handler != NULL)
		drv_led.handler();
}
