/*
 @autor:       Alexandr Haidamaka
 @file:        drv_dimmer.c
 @description: driver dimmer functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
drv_dimmer_t drv_dimmer[NUM_DRV_DIMMER];

//------------------------------------------------------------------------------
void drv_dimmer_init(void)
{
//	int i, j, end;
//
//	for (i = 0, j = 0, end = NUM_IOT_BT; i < end; i++, j++)
//	{
//		drv_bt[i].pin = iot_bt[j].in;
//		drv_bt[i].state = pin_read(drv_bt[i].pin);
//		drv_bt[i].off_callback = NULL;
//		drv_bt[i].on_callback = iot_button_on_callback;
//		drv_bt[i].iot_ind = j;
//	}
//
//	for (j = 0, end += NUM_IOT_BT_RELAY; i < end; i++, j++)
//	{
//		drv_bt[i].pin = iot_bt_relay[j].pin.in;
//		drv_bt[i].state = pin_read(drv_bt[i].pin);
//		drv_bt[i].off_callback = iot_button_relay_off_callback;
//		drv_bt[i].on_callback = iot_button_relay_on_callback;
//		drv_bt[i].iot_ind = j;
//	}
}

//------------------------------------------------------------------------------
void drv_dimmer_handler()
{
//	for (int i = 0; i < NUM_DRV_BT; i++)
//	{
//		bool state = pin_read(drv_bt[i].pin);
//
//		if (state != drv_bt[i].state)
//		{
//			drv_bt[i].state = state;
//
//			switch (state)
//			{
//			case false: // button push-up
//				if (drv_bt[i].on_callback != NULL)
//					drv_bt[i].on_callback(drv_bt[i].iot_ind);
//				break;
//			case true: // button push-down
//				if (drv_bt[i].off_callback != NULL)
//					drv_bt[i].off_callback(drv_bt[i].iot_ind);
//				break;
//			}
//		}
//	}
}
