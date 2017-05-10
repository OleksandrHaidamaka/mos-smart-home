/*
 @autor:       Alexandr Haidamaka
 @file:        button.c
 @description: button driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
input_t bt_driver[NUM_BT_DRIVER];

//------------------------------------------------------------------------------
void button_driver_init(void)
{
	int i, end;

	for (i = 0, end = NUM_BT_IOT; i < end; i++)
	{
		bt_driver[i].pin = bt_relay[i].pin.in;
	}

	for (end += NUM_BT_RELAY_IOT; i < end; i++)
	{
		bt_driver[i].pin = bt_relay[i].pin.in;
	}
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_BT_DRIVER; i++)
	{
		bool state = pin_read(bt_driver[i].pin);

		if (state != bt_driver[i].state)
		{
			bt_driver[i].state = state;

			switch (state)
			{
			case false: // button push-up
				if (bt_driver[i].on_callback != NULL)
					bt_driver[i].on_callback(i);
				break;
			case true: // button push-down
				if (bt_driver[i].off_callback != NULL)
					bt_driver[i].off_callback(i);
				break;
			}
		}
	}
}
