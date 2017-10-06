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
		pin_output(iot_relay[i].pin.out);
		pin_write(iot_relay[i].pin.out, true);
		iot_relay[i].mode.current = NORMAL_MODE;
		iot_bt_relay[i].mode.task.handler = NULL;
	}
}

//------------------------------------------------------------------------------
void iot_relay_handler(void)
{
	for (int i = 0; i < NUM_IOT_RELAY; i++)
	{
		//выполнение задачи режима
		if (iot_relay[i].mode.task.handler != NULL)
			iot_relay[i].mode.task.handler((void *) &iot_relay[i]);
	}
}
