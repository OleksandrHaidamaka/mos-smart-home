/*
 @autor:       Alexandr Haidamaka
 @file:        sw_relay.c
 @description: switch-relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void switch_relay_init(void)
{
	for (int i = 0; i < NUM_SW_RELAY_IOT; i++)
	{
		pin_input(sw_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(sw_relay[i].pin.out);
		pin_write(sw_relay[i].pin.out, pin_read(sw_relay[i].pin.in));
	}
}

//------------------------------------------------------------------------------
void switch_relay_off_callback(int i)
{
	pin_write(sw_relay[i].pin.out, true);
	sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void switch_relay_on_callback(int i)
{
	pin_write(sw_relay[i].pin.out, false);
	sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
