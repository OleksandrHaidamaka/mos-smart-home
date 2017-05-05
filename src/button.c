/*
 @autor:       Alexandr Haidamaka
 @file:        button.c
 @description: button driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  NUM_BT_DRIVER       (NUM_BT_IOT + NUM_BT_RELAY_IOT)

#define  BT_RELAY_OFFSET     (NUM_BT_IOT)
int bt_driver_pin_map[NUM_BT_DRIVER];

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
input_t bt_diver[NUM_BT_DRIVER];

bt_relay_t bt_relay[NUM_BT_RELAY_IOT]; // button_relay object

static void button_relay_on_callback(int i);

void button_driver_init(void)
{
	int i, end;

	for (i = 0, end = NUM_BT_IOT; i < end; i++)
	{
		bt_driver_pin_map[i] = bt_iot_pin_map[i].in;
	}

	for (end += NUM_BT_RELAY_IOT; i < end; i++)
	{
		bt_driver_pin_map[i] = bt_relay_iot_pin_map[i].in;
	}
}

//------------------------------------------------------------------------------
static void bt_relay_periph(void)
{
	for (int i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		pin_input_up(bt_relay_iot_pin_map[i].in);
		pin_output(bt_relay_iot_pin_map[i].out);
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

		bt_relay[i].mqtt_update = true;
		pin_write(bt_relay_iot_pin_map[i].out, true);
	}
}

//------------------------------------------------------------------------------
void button_relay_on_callback(int i)
{
	i -= BT_RELAY_OFFSET;
	pin_write(bt_relay_iot_pin_map[i].out, !pin_read(bt_relay_iot_pin_map[i].out));
	bt_relay[i].mqtt_update = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_BT_DRIVER; i++)
	{
		bool state = pin_read(bt_driver_pin_map[i]);

		if (state != bt_diver[i].state)
		{
			bt_diver[i].state = state;

			switch (state)
			{
			case false: // button push-up
				if (bt_diver[i].on_callback != NULL) bt_diver[i].on_callback(i);
				break;
			case true: // button push-down
				if (bt_diver[i].off_callback != NULL) bt_diver[i].off_callback(i);
				break;
			}
		}
	}
}
