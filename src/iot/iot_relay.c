/*
 @autor:       Alexandr Haidamaka
 @file:        relay.c
 @description: relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void relay_init(void)
{
	for (int i = 0; i < NUM_RELAY_IOT; i++)
	{
		pin_output(relay[i].out);
		pin_write(relay[i].out, true);
	}
}
