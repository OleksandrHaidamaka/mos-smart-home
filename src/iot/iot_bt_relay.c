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
void iot_button_relay_mode_task_handler(int i, void (*handler)(int))
{
	iot_bt_relay[i].mode.task.handler = handler;
	iot_bt_relay[i].mode.task.count = 0;
	iot_bt_relay[i].mode.task.state = 0;
	iot_bt_relay[i].mode.task.time = 0;
}

//------------------------------------------------------------------------------
int iot_button_task_blink(int i, int delay)
{
	if (iot_bt_relay[i].mode.task.time != 0)
	{
		iot_bt_relay[i].mode.task.time--;
		return 0;
	}

	switch (iot_bt_relay[i].mode.task.state)
	{
	case 0: // off relay
		pin_write(iot_bt_relay[i].pin.out, true);
		iot_bt_relay[i].mode.task.state = 1;
		iot_bt_relay[i].mode.task.time = delay;
		break;
	case 1: // on relay
		pin_write(iot_bt_relay[i].pin.out, false);
		iot_bt_relay[i].mode.task.state = 0;
		iot_bt_relay[i].mode.task.time = delay;
		break;
	}

	return 1;
}

//------------------------------------------------------------------------------
void iot_button_relay_task_off_on_alarm(int i)
{
	if (iot_button_task_blink(i, SHORT_TASK_DELAY) == 0)
		return;

	if (iot_bt_relay[i].mode.task.count++ > 10)
	{
		iot_button_relay_mode_task_handler(i, NULL);
		pin_write(iot_bt_relay[i].pin.out, iot_bt_relay[i].mode.pin_state);
		iot_bt_relay[i].mqtt = POLL;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_task_sos(int i)
{
	iot_button_task_blink(i, SOS_TASK_DELAY);
}

//------------------------------------------------------------------------------
void iot_button_relay_task_panic(int i)
{
	iot_button_task_blink(i, PANIC_TASK_DELAY);
}

//------------------------------------------------------------------------------
void iot_button_relay_init(void)
{
	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		pin_input(iot_bt_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(iot_bt_relay[i].pin.out);
		pin_write(iot_bt_relay[i].pin.out, true);
		iot_bt_relay[i].bt_handler = NULL;
		iot_bt_relay[i].mode.current = iot_bt_relay[i].mode.requested = NORMAL_MODE;
		iot_bt_relay[i].mode.task.handler = NULL;
	}
}

//------------------------------------------------------------------------------
void bt_handler_normal_mode(int i)
{
	iot_bt_relay[i].mode.timer++;
	if (iot_bt_relay[i].mode.timer > ALARM_PANIC_MODE_DELAY)
	{
		pin_write(iot_bt_relay[i].pin.out, !pin_read(iot_bt_relay[i].pin.out));
		iot_bt_relay[i].mode.requested = ALARM_MODE;
		iot_bt_relay[i].mode.long_press = true;
		iot_bt_relay[i].bt_handler = NULL;
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
		iot_bt_relay[i].bt_handler = bt_handler_normal_mode;
		iot_bt_relay[i].mqtt = EVENT;
		break;
	case SOS_MODE:
		iot_bt_relay[i].mode.requested = NORMAL_MODE;
		iot_bt_relay[i].bt_handler = NULL;
		iot_bt_relay[i].mqtt = EVENT;
		break;
	case ALARM_MODE:
	case PANIC_MODE:
		if (iot_bt_relay[i].mode.timer > ALARM_PANIC_MODE_DELAY)
		{
			iot_bt_relay[i].mode.requested = NORMAL_MODE;
			iot_bt_relay[i].mode.long_press = true;
			iot_bt_relay[i].bt_handler = NULL;
			iot_bt_relay[i].mqtt = EVENT;
		}
		break;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_off_callback(int i)
{
	iot_bt_relay[i].bt_handler = NULL;

	if (iot_bt_relay[i].mode.long_press == false)
	{
		iot_bt_relay[i].mqtt = EVENT;
		switch ((int) iot_bt_relay[i].mode.current)
		{
		case ALARM_MODE:
			pin_write(iot_bt_relay[i].pin.out, !pin_read(iot_bt_relay[i].pin.out));
			iot_bt_relay[i].mode.requested = iot_bt_relay[i].mode.current;
			break;
		case PANIC_MODE:
			iot_bt_relay[i].mode.requested = iot_bt_relay[i].mode.current;
			break;
		}
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback(int i)
{
	iot_bt_relay[i].mode.timer = 0;
	iot_bt_relay[i].mode.long_press = false;
	iot_bt_relay[i].bt_handler = iot_button_relay_on_callback_handler;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void iot_button_relay_handler(void)
{
	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		//пост обработка кнопки
		if (iot_bt_relay[i].bt_handler != NULL)
			iot_bt_relay[i].bt_handler(i);

		//выполнение задачи режима
		if (iot_bt_relay[i].mode.task.handler != NULL)
			iot_bt_relay[i].mode.task.handler(i);
	}
}
