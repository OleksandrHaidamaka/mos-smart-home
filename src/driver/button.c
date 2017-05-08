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
 *** DEFENITIONS
 ******************************************************************************/

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int bt_driver_pin_map[NUM_BT_DRIVER];
input_t bt_diver[NUM_BT_DRIVER];

void button_driver_init(void)
{
	int i, end;

	for (i = 0, end = NUM_BT_IOT; i < end; i++)
	{
		bt_driver_pin_map[i] = bt_iot_pin_map[i].in;
	}

	for (end += NUM_BT_RELAY_IOT; i < end; i++)
	{
		bt_driver_pin_map[i] = bt_relay_iot_pin_map[i].in;
	}
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_BT_DRIVER; i++)
	{
		bool state = pin_read(bt_driver_pin_map[i]);

		if (state != bt_diver[i].state)
		{
			bt_diver[i].state = state;

			switch (state)
			{
			case false: // button push-up
				if (bt_diver[i].on_callback != NULL) bt_diver[i].on_callback(i);
				break;
			case true: // button push-down
				if (bt_diver[i].off_callback != NULL) bt_diver[i].off_callback(i);
				break;
			}
		}
	}
}
