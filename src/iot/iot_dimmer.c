/*
 @autor:       Alexandr Haidamaka
 @file:        iot_dimmer.c
 @description: iot dimmer functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_dimmer_init(void)
{
	pin_input(iot_dimmer.pin_zero, MGOS_GPIO_PULL_NONE);

	for (int i = 0; i < NUM_IOT_DIMMER; i++)
	{
		pin_output(iot_dimmer.sub[i].pin_phase);
		pin_write(iot_dimmer.sub[i].pin_phase, false);
	}
}
