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
	int i, j, end;

	for (i = 0, j = 0, end = NUM_BT_IOT; i < end; i++, j++)
	{
		bt_driver[i].pin = bt_relay[i].pin.in;
		bt_driver[i].iot_ind = j;
	}

	for (j = 0, end += NUM_BT_RELAY_IOT; i < end; i++, j++)
	{
		bt_driver[i].pin = bt_relay[j].pin.in;
		bt_driver[i].state = pin_read(bt_driver[i].pin);
		bt_driver[i].off_callback = NULL;
		bt_driver[i].on_callback = button_relay_on_callback;
		bt_driver[i].iot_ind = j;
	}
}

//------------------------------------------------------------------------------
void button_driver_handler()
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
					bt_driver[i].on_callback(bt_driver[i].iot_ind);
				break;
			case true: // button push-down
				if (bt_driver[i].off_callback != NULL)
					bt_driver[i].off_callback(bt_driver[i].iot_ind);
				break;
			}
		}
	}
}
