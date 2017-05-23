/*
 @autor:       Alexandr Haidamaka
 @file:        iot_sw_relay.c
 @description: iot switch-relay functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_switch_relay_init(void)
{
	for (int i = 0; i < NUM_IOT_SW_RELAY; i++)
	{
		pin_input(iot_sw_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(iot_sw_relay[i].pin.out);
		pin_write(iot_sw_relay[i].pin.out, pin_read(iot_sw_relay[i].pin.in));
	}
}

//------------------------------------------------------------------------------
void iot_switch_relay_off_callback(int i)
{
	pin_write(iot_sw_relay[i].pin.out, true);
	iot_sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void iot_switch_relay_on_callback(int i)
{
	pin_write(iot_sw_relay[i].pin.out, false);
	iot_sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
