/*
 @autor:       Alexandr Haidamaka
 @file:        iot_bt_relay.c
 @description: iot button-relay functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define GO_TO_NORMAL_TASK_DELAY  (50 / SYS_TICK)
#define SOS_PANIC_TASK_DELAY     (250 / SYS_TICK)
#define ALARM_PANIC_MODE_DELAY   (3000 / SYS_TICK)

//------------------------------------------------------------------------------
void iot_button_relay_mode_task_handler(int i, void (*handler)(int))
{
	iot_bt_relay[i].mode.task.count = 0;
	iot_bt_relay[i].mode.task.state = 0;
	iot_bt_relay[i].mode.task.time = 0;
	iot_bt_relay[i].mode.task.handler = handler;
}

//------------------------------------------------------------------------------
void iot_button_relay_task_go_to_normal(int i)
{
	if (iot_bt_relay[i].mode.task.time != 0)
	{
		iot_bt_relay[i].mode.task.time--;
		return;
	}

	if (iot_bt_relay[i].mode.task.count++ > 6)
	{
		iot_button_relay_mode_task_handler(i, NULL);
		pin_write(iot_bt_relay[i].pin.out, iot_bt_relay[i].mode.pin_state);
		iot_bt_relay[i].mqtt = true;
	}

	switch (iot_bt_relay[i].mode.task.state)
	{
	case 0: // off relay
		pin_write(iot_bt_relay[i].pin.out, true);
		iot_bt_relay[i].mode.task.state = 1;
		iot_bt_relay[i].mode.task.time = GO_TO_NORMAL_TASK_DELAY;
		break;
	case 1: // on relay
		pin_write(iot_bt_relay[i].pin.out, false);
		iot_bt_relay[i].mode.task.state = 0;
		iot_bt_relay[i].mode.task.time = GO_TO_NORMAL_TASK_DELAY;
		break;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_task_sos_panic(int i)
{
	if (iot_bt_relay[i].mode.task.time != 0)
	{
		iot_bt_relay[i].mode.task.time--;
		return;
	}

	switch (iot_bt_relay[i].mode.task.state)
	{
	case 0: // off relay
		pin_write(iot_bt_relay[i].pin.out, true);
		iot_bt_relay[i].mode.task.state = 1;
		iot_bt_relay[i].mode.task.time = SOS_PANIC_TASK_DELAY;
		break;
	case 1: // on relay
		pin_write(iot_bt_relay[i].pin.out, false);
		iot_bt_relay[i].mode.task.state = 0;
		iot_bt_relay[i].mode.task.time = SOS_PANIC_TASK_DELAY;
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
		iot_bt_relay[i].mode.name = NORMAL_MODE;
		iot_bt_relay[i].mode.task.handler = NULL;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_off_callback(int i)
{
	if (iot_bt_relay[i].mode.long_press == false)
	{
		switch ((int) iot_bt_relay[i].mode.name)
		{
		case ALARM_MODE:
		case PANIC_MODE:
			iot_bt_relay[i].mqtt = true;
			break;
		}
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback(int i)
{
	printf("%s(%d)\n", __func__, i);
	iot_bt_relay[i].mode.timer = 0;
	iot_bt_relay[i].mode.long_press = false;
}

inline static void iot_button_relay_off_on_callback_handler()
{
	int i;

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		if (iot_bt_relay[i].mode.long_press == false)
		{
			iot_bt_relay[i].mode.timer++;
			switch ((int) iot_bt_relay[i].mode.name)
			{
			case NORMAL_MODE:
				iot_bt_relay[i].mode.long_press = true;
				iot_bt_relay[i].mqtt = true;
				pin_write(iot_bt_relay[i].pin.out,
						!pin_read(iot_bt_relay[i].pin.out));
				break;
			case SOS_MODE:
				iot_bt_relay[i].mode.long_press = true;
				iot_bt_relay[i].mqtt = true;
				break;
			case ALARM_MODE:
			case PANIC_MODE:
				if (iot_bt_relay[i].mode.timer > ALARM_PANIC_MODE_DELAY)
				{
					iot_bt_relay[i].mode.long_press = true;
					iot_bt_relay[i].mqtt = true;
				}
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_handler(void)
{
	iot_button_relay_off_on_callback_handler();

	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		if (iot_bt_relay[i].mode.task.handler != NULL)
			iot_bt_relay[i].mode.task.handler(i);
	}
}
