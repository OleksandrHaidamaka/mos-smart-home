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
#define LED_PIN            (D4)
#define led_off()          pin_write(LED_PIN, true)
#define led_on()           pin_write(LED_PIN, false)

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
struct blink_mode_t main_blink_mode[] =
{
//       time short, time long, number of short blinks, repeat flag
		{ 250 / SYS_TICK, 1000 / SYS_TICK, 1, true }, // BL_WIFI_DISCONNECTED
		{ 250 / SYS_TICK, 1000 / SYS_TICK, 2, true }, // BL_WIFI_IP_ACQUIRED
		{ 250 / SYS_TICK, 1000 / SYS_TICK, 3, true }, // BL_MQTT_CONNECTED
		{ 50 / SYS_TICK, 1000 / SYS_TICK, 1, false }, // BL_MQTT_SUB_MSG_ERR
		{ 50 / SYS_TICK, 1000 / SYS_TICK, 2, false }, // BL_MQTT_SUB_MSG_OK
		{ 50 / SYS_TICK, 1000 / SYS_TICK, 2, false }  // BL_MQTT_PUB_MSG
};

enum bl_mode bl_mode_current = BL_WIFI_DISCONNECTED;
enum bl_mode bl_mode_new = BL_WIFI_DISCONNECTED;

//------------------------------------------------------------------------------
static void led_periph()
{
	pin_output(LED_PIN);
}

//------------------------------------------------------------------------------
void led_init()
{
	led_periph();
	led_off();
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
	static bool go_to_current_mode = false;

	if (time != 0)
		time--;

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count >= main_blink_mode[mode].times)
			{
				led_off();
				count = 0;
				time = main_blink_mode[mode].time_long;

				mode = bl_mode_new;

				if (go_to_current_mode == true)
				{
					go_to_current_mode = false;
					mode = bl_mode_new = bl_mode_current;
				}
				if (main_blink_mode[mode].repeat == false)
					go_to_current_mode = true;

				break;
			}
			led_on();
			time = main_blink_mode[mode].time_short;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_off();
			time = main_blink_mode[mode].time_short;
			state--;
		}
		break;
	}
}
