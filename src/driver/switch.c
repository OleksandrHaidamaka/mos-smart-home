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
	int i, end;

	for (i = 0, end = NUM_SW_IOT; i < end; i++)
	{
		sw_driver[i].pin = sw_relay[i].pin.in;
	}

	for (end += NUM_SW_RELAY_IOT; i < end; i++)
	{
		sw_driver[i].pin = sw_relay[i].pin.in;
	}
}

//------------------------------------------------------------------------------
void switch_driver()
{
	for (int i = 0; i < NUM_SW_DRIVER; i++)
	{
		bool state = pin_read(sw_driver[i].pin);

		if (state != sw_driver[i].state)
		{
			sw_driver[i].state = state;

			switch (state)
			{
			case false: // button push-up
				if (sw_driver[i].on_callback != NULL)
					sw_driver[i].on_callback(i);
				break;
			case true: // button push-down
				if (sw_driver[i].off_callback != NULL)
					sw_driver[i].off_callback(i);
				break;
			}
		}
	}
}
