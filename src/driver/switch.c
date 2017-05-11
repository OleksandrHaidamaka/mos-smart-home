/*
 @autor:       Alexandr Haidamaka
 @file:        switch.c
 @description: Switch driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
input_t sw_driver[NUM_SW_DRIVER];

//------------------------------------------------------------------------------
void switch_driver_init(void)
{
	int i, j, end;

	for (i = 0, j = 0, end = NUM_SW_IOT; i < end; i++, j++)
	{
		sw_driver[i].pin = sw_relay[i].pin.in;
		sw_driver[i].iot_ind = j;
	}

	for (j = 0, end += NUM_SW_RELAY_IOT; i < end; i++, j++)
	{
		sw_driver[i].pin = sw_relay[j].pin.in;
		sw_driver[i].state = pin_read(sw_driver[i].pin);
		sw_driver[i].off_callback = switch_relay_off_callback;
		sw_driver[i].on_callback = switch_relay_on_callback;
		sw_driver[i].iot_ind = j;
	}
}

//------------------------------------------------------------------------------
void switch_driver_handler()
{
	for (int i = 0; i < NUM_SW_DRIVER; i++)
	{
		bool state = pin_read(sw_driver[i].pin);

		if (state != sw_driver[i].state)
		{
			sw_driver[i].state = state;

			switch (state)
			{
			case false: // switch on
				if (sw_driver[i].on_callback != NULL)
					sw_driver[i].on_callback(sw_driver[i].iot_ind);
				break;
			case true: // switch off
				if (sw_driver[i].off_callback != NULL)
					sw_driver[i].off_callback(sw_driver[i].iot_ind);
				break;
			}
		}
	}
}
