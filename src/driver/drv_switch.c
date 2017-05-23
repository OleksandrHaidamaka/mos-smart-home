/*
 @autor:       Alexandr Haidamaka
 @file:        drv_switch.c
 @description: driver switch functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
drv_input_t drv_sw[NUM_DRV_SW];

//------------------------------------------------------------------------------
void drv_switch_init(void)
{
	int i, j, end;

	for (i = 0, j = 0, end = NUM_IOT_SW; i < end; i++, j++)
	{
		drv_sw[i].pin = iot_sw[j].in;
		drv_sw[i].state = pin_read(drv_sw[i].pin);
		drv_sw[i].off_callback = iot_switch_off_callback;
		drv_sw[i].on_callback = iot_switch_on_callback;
		drv_sw[i].iot_ind = j;
	}

	for (j = 0, end += NUM_IOT_SW_RELAY; i < end; i++, j++)
	{
		drv_sw[i].pin = iot_sw_relay[j].pin.in;
		drv_sw[i].state = pin_read(drv_sw[i].pin);
		drv_sw[i].off_callback = iot_switch_relay_off_callback;
		drv_sw[i].on_callback = iot_switch_relay_on_callback;
		drv_sw[i].iot_ind = j;
	}
}

//------------------------------------------------------------------------------
void drv_switch_handler()
{
	for (int i = 0; i < NUM_DRV_SW; i++)
	{
		bool state = pin_read(drv_sw[i].pin);

		if (state != drv_sw[i].state)
		{
			drv_sw[i].state = state;

			switch (state)
			{
			case false: // switch on
				if (drv_sw[i].on_callback != NULL)
					drv_sw[i].on_callback(drv_sw[i].iot_ind);
				break;
			case true: // switch off
				if (drv_sw[i].off_callback != NULL)
					drv_sw[i].off_callback(drv_sw[i].iot_ind);
				break;
			}
		}
	}
}
