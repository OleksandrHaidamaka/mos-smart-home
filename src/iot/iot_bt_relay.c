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
#define SOS_TASK_DELAY  (500 / SYS_TICK)
#define ALARM_DELAY     (3000 / SYS_TICK)

//------------------------------------------------------------------------------
static void sos_task(int i)
{
	static int state = 0;
	static int time = SOS_TASK_DELAY;

	if (--time != 0)
		return;

	switch (state)
	{
	case 0: // off relay
		pin_write(iot_bt_relay[i].pin.out, true);
		state = 1;
		time = SOS_TASK_DELAY;
		break;
	case 1: // on relay
		pin_write(iot_bt_relay[i].pin.out, false);
		state = 0;
		time = SOS_TASK_DELAY;
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
void iot_button_relay_off_callback(int i)
{
	iot_bt_relay[i].alarm_time = 0;
	iot_bt_relay[i].alarm_time_en = false;
}

//------------------------------------------------------------------------------
void iot_button_relay_on_callback(int i)
{
	printf("%s(%d)\n", __func__, i);

	switch ((int) iot_bt_relay[i].mode)
	{
	case NORMAL_MODE:
		iot_bt_relay[i].mqtt = true;
		pin_write(iot_bt_relay[i].pin.out, !pin_read(iot_bt_relay[i].pin.out));
		break;
	case DISCO_MODE:
	case SOS_MODE:
		iot_bt_relay[i].mqtt = true;
		break;
	case ALARM_MODE:
		iot_bt_relay[i].alarm_time = 0;
		iot_bt_relay[i].alarm_time_en = true;
		break;
	}
}

//------------------------------------------------------------------------------
void iot_button_relay_handler(void)
{
	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
		if (iot_bt_relay[i].mode == ALARM_MODE)
			if (iot_bt_relay[i].alarm_time_en == true)
				if (iot_bt_relay[i].alarm_time++ > ALARM_DELAY)
				{
					iot_bt_relay[i].alarm_time = 0;
					iot_bt_relay[i].alarm_time_en = false;
					iot_bt_relay[i].mqtt = true;
				}

	for (int i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		switch ((int) iot_bt_relay[i].mode)
		{
		case DISCO_MODE:
		case SOS_MODE:
		case ALARM_MODE:
			sos_task(i);
			break;
		}
	}
}
