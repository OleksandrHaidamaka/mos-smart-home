/*
 @autor:       Alexandr Haidamaka
 @file:        iot.c
 @description: iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

//------------------------------------------------------------------------------
void iot_x_relay_mode_task_handler(iot_x_relay_t* iot, void (*handler)(int))
{
	iot->mode.task.handler = handler;
	iot->mode.task.count = 0;
	iot->mode.task.state = 0;
	iot->mode.task.time = 0;
}

//------------------------------------------------------------------------------
static int iot_x_relay_task_blink(iot_x_relay_t* iot, int delay)
{
	if (iot->mode.task.time != 0)
	{
		iot->mode.task.time--;
		return 0;
	}

	switch (iot->mode.task.state)
	{
	case 0: // off relay
		pin_write(iot->pin.out, true);
		iot->mode.task.state = 1;
		iot->mode.task.time = delay;
		break;
	case 1: // on relay
		pin_write(iot->pin.out, false);
		iot->mode.task.state = 0;
		iot->mode.task.time = delay;
		break;
	}
	return 1;
}

//------------------------------------------------------------------------------
void iot_x_relay_task_sos(iot_x_relay_t* iot)
{
	iot_x_relay_task_blink(iot, SOS_TASK_DELAY);
}

//------------------------------------------------------------------------------
void iot_x_relay_task_off_on_alarm(iot_x_relay_t* iot)
{
	if (iot_x_relay_task_blink(iot, SHORT_TASK_DELAY) == 0)
		return;

	if (iot->mode.task.count++ > 10)
	{
		iot_x_relay_mode_task_handler(iot, NULL);
		pin_write(iot->pin.out, iot->mode.pin_state);
		iot->mqtt = POLL;
	}
}

//------------------------------------------------------------------------------
//void iot_button_relay_task_panic(int i)
//{
//	iot_button_task_blink(i, PANIC_TASK_DELAY);
//}

