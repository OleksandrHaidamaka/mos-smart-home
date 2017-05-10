/*
 @autor:       Alexandr Haidamaka
 @file:        bt_relay.c
 @description: button-relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  BT_RELAY_OFFSET  (NUM_BT_IOT)

/*******************************************************************************
 *** PRIVATE PROTOTYPES
 ******************************************************************************/
static void button_relay_on_callback(int i);

//------------------------------------------------------------------------------
static void bt_relay_periph(void)
{
	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		pin_input(bt_relay[i].pin.in, MGOS_GPIO_PULL_UP);
		pin_output(bt_relay[i].pin.out);
	}
}

//------------------------------------------------------------------------------
void button_relay_init(void)
{
	bt_relay_periph();

	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		bt_driver[i + BT_RELAY_OFFSET].off_callback = NULL;
		bt_driver[i + BT_RELAY_OFFSET].on_callback = button_relay_on_callback;
		bt_driver[i + BT_RELAY_OFFSET].state = pin_read(bt_driver[i].pin);
		pin_write(bt_relay[i].pin.out, true);
	}
}

//------------------------------------------------------------------------------
static void button_relay_on_callback(int i)
{
	i -= BT_RELAY_OFFSET;
	pin_write(bt_relay[i].pin.out, !pin_read(bt_relay[i].pin.out));
	bt_relay[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
