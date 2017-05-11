/*
 @autor:       Alexandr Haidamaka
 @file:        bt_relay.c
 @description: button-relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void button_relay_init(void)
{
	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		pin_input(bt_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(bt_relay[i].pin.out);
		pin_write(bt_relay[i].pin.out, true);
	}
}

//------------------------------------------------------------------------------
void button_relay_on_callback(int i)
{
	pin_write(bt_relay[i].pin.out, !pin_read(bt_relay[i].pin.out));
	bt_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
