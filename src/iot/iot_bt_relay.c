/*
 @autor:       Alexandr Haidamaka
 @file:        iot_bt_relay.c
 @description: iot button-relay functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_button_relay_init(void)
{
	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		pin_input(iot_bt_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(iot_bt_relay[i].pin.out);
		pin_write(iot_bt_relay[i].pin.out, true);
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback(int i)
{
	pin_write(iot_bt_relay[i].pin.out, !pin_read(iot_bt_relay[i].pin.out));
	iot_bt_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
