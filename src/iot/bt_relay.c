/*
 @autor:       Alexandr Haidamaka
 @file:        bt_relay.c
 @description: button-relay iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  BT_RELAY_OFFSET     (NUM_BT_IOT)

/*******************************************************************************
 *** PRIVATE PROTOTYPES
 ******************************************************************************/
static void button_relay_on_callback(int i);

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/

//------------------------------------------------------------------------------
static void bt_relay_periph(void)
{
	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		pin_input_up(bt_relay[i].pin_map.in);
		pin_output(bt_relay[i].pin_map.out);
	}
}

//------------------------------------------------------------------------------
void button_relay_init(void)
{
	bt_relay_periph();

	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		bt_diver[i + BT_RELAY_OFFSET].on_callback = button_relay_on_callback;
		bt_diver[i + BT_RELAY_OFFSET].off_callback = NULL;
		bt_diver[i + BT_RELAY_OFFSET].state = pin_read(bt_driver_pin_map[i]);
		pin_write(bt_relay[i].pin_map.out, true);
	}
}

//------------------------------------------------------------------------------
void button_relay_on_callback(int i)
{
	i -= BT_RELAY_OFFSET;
	pin_write(bt_relay[i].pin_map.out, !pin_read(bt_relay[i].pin_map.out));
	bt_relay[i].mqtt_update = true;
	printf("%s(%d)\n", __func__, i);
}
