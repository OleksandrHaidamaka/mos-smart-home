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
static void sos_task()
{
	int i;
	static int state = 0;
	static int time = 500 / SYS_TICK;

	if (--time != 0)
		return;

	switch (state)
	{
	case 0: // off relay
		for (i = 0; i < NUM_IOT_BT_RELAY; i++)
			pin_write(iot_bt_relay[i].pin.out, true);
		state = 1;
		time = 500 / SYS_TICK;
		break;
	case 1: // on relay
		for (i = 0; i < NUM_IOT_BT_RELAY; i++)
			pin_write(iot_bt_relay[i].pin.out, false);
		state = 0;
		time = 500 / SYS_TICK;
		break;
	}
}

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

void iot_button_relay_handler(void)
{
	sos_task();
}
