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
	for (int i = 0; i < NUM_IOT_DIMMER; i++)
	{
		pin_input(iot_dimmer[i].pin.in, MGOS_GPIO_PULL_NONE);
		pin_output(iot_dimmer[i].pin.out);
		pin_write(iot_dimmer[i].pin.out, false);
	}
}
