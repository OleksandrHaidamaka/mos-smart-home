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
		iot_bt_relay[i].handler = NULL;
		iot_bt_relay[i].mode.pin_state = true;
		iot_bt_relay[i].mode.current = iot_bt_relay[i].mode.requested = NORMAL_MODE;
		iot_bt_relay[i].mode.task.handler = NULL;
		iot_bt_relay[i].mqtt = POLL;
	}
}

//------------------------------------------------------------------------------
void bt_handler_normal_mode(int i)
{
	iot_bt_relay[i].mode.timer++;
	if (iot_bt_relay[i].mode.timer > ALARM_PANIC_MODE_DELAY)
	{
		iot_bt_relay[i].mode.requested = ALARM_MODE;
		iot_bt_relay[i].mode.long_press = true;
		iot_bt_relay[i].handler = NULL;
		iot_bt_relay[i].mqtt = EVENT;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback_handler(int i)
{
	iot_bt_relay[i].mode.timer++;
	switch ((int) iot_bt_relay[i].mode.current)
	{
	case NORMAL_MODE:
		pin_write(iot_bt_relay[i].pin.out, !pin_read(iot_bt_relay[i].pin.out));
		iot_bt_relay[i].handler = bt_handler_normal_mode;
		iot_bt_relay[i].mqtt = EVENT;
		break;
	case SOS_MODE: case SENSOR_MODE:
		iot_bt_relay[i].mode.requested = NORMAL_MODE;
		iot_bt_relay[i].handler = NULL;
		iot_bt_relay[i].mqtt = EVENT;
		break;
	case ALARM_MODE:
	case PANIC_MODE:
		if (iot_bt_relay[i].mode.timer > ALARM_PANIC_MODE_DELAY)
		{
			iot_bt_relay[i].mode.requested = NORMAL_MODE;
			iot_bt_relay[i].mode.long_press = true;
			iot_bt_relay[i].handler = NULL;
			iot_bt_relay[i].mqtt = EVENT;
		}
		break;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_off_callback(int i)
{
	iot_bt_relay[i].handler = NULL;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback(int i)
{
	iot_bt_relay[i].mode.timer = 0;
	iot_bt_relay[i].mode.long_press = false;
	iot_bt_relay[i].handler = iot_button_relay_on_callback_handler;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void iot_button_relay_handler(void)
{
	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		//пост обработка кнопки
		if (iot_bt_relay[i].handler != NULL)
			iot_bt_relay[i].handler(i);

		//выполнение задачи режима
		if (iot_bt_relay[i].mode.task.handler != NULL)
			iot_bt_relay[i].mode.task.handler((void *) &iot_bt_relay[i]);
	}
}
