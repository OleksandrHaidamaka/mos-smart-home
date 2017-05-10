/*
 @autor:       Alexandr Haidamaka
 @file:        sw_relay.c
 @description: switch-relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  SW_RELAY_OFFSET  (NUM_SW_IOT)

/*******************************************************************************
 *** PRIVATE PROTOTYPES
 ******************************************************************************/
static void switch_relay_off_callback(int i);
static void switch_relay_on_callback(int i);

//------------------------------------------------------------------------------
static void sw_relay_periph(void)
{
	for (int i = 0; i < NUM_SW_RELAY_IOT; i++)
	{
		pin_input(sw_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(sw_relay[i].pin.out);
	}
}

//------------------------------------------------------------------------------
void switch_relay_init(void)
{
	sw_relay_periph();

	for (int i = 0; i < NUM_SW_RELAY_IOT; i++)
	{
		sw_driver[i + SW_RELAY_OFFSET].off_callback = switch_relay_off_callback;
		sw_driver[i + SW_RELAY_OFFSET].on_callback = switch_relay_on_callback;
		sw_driver[i + SW_RELAY_OFFSET].state = pin_read(sw_driver[i].pin);
		pin_write(sw_relay[i].pin.out, true);
	}
}

//------------------------------------------------------------------------------
static void switch_relay_off_callback(int i)
{
	i -= SW_RELAY_OFFSET;
	pin_write(sw_relay[i].pin.out, true);
	sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void switch_relay_on_callback(int i)
{
	i -= SW_RELAY_OFFSET;
	pin_write(sw_relay[i].pin.out, false);
	sw_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
