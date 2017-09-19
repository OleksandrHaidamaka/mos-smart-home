/*
 @autor:       Alexandr Haidamaka
 @file:        iot_relay.c
 @description: iot relay functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_relay_init(void)
{
	for (int i = 0; i < NUM_IOT_RELAY; i++)
	{
		pin_output(iot_relay[i].out);
		pin_write(iot_relay[i].out, true);
		iot_relay[i].mode.current = NORMAL_MODE;
	}
}
