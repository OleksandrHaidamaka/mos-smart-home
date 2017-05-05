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
#define  NUM_BT_DRIVER       (NUM_BT_O + NUM_BT_RELAY_O)

#if (NUM_BT_DRIVER > 0)
int bt_driver_pin_map[NUM_BT_DRIVER] =
{ D1, D2 };
#endif

#if (NUM_BT_RELAY_O > 0)
in_out_t bt_relay_pin_map[NUM_BT_RELAY_O] =
{
{ .in = D1, .out = D5 },
{ .in = D2, .out = D6 } };
#endif

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
input_t bt_diver[NUM_BT_DRIVER];

bt_relay_t bt_relay[NUM_BT_RELAY_O]; // button_relay object

static void button_relay_on_callback(int i);

//------------------------------------------------------------------------------
static void bt_relay_periph(void)
{
	for (int i = 0; i < NUM_BT_RELAY_O; i++)
	{
		pin_input_up(bt_relay_pin_map[i].in);
		pin_output(bt_relay_pin_map[i].out);
	}
}

//------------------------------------------------------------------------------
void button_relay_init(void)
{
	bt_relay_periph();

	for (int i = 0; i < NUM_BT_RELAY_O; i++)
	{
		bt_diver[i].on_callback = button_relay_on_callback; // переиндексацию сделать
		bt_diver[i].off_callback = NULL;
		bt_diver[i].state = pin_read(bt_driver_pin_map[i]);

		bt_relay[i].mqtt_update = true;
		pin_write(bt_relay_pin_map[i].out, true);
	}
}

//------------------------------------------------------------------------------
void button_relay_on_callback(int i)
{
	pin_write(bt_relay_pin_map[i].out, !pin_read(bt_relay_pin_map[i].out));
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
				if (bt_diver[i].on_callback != NULL)
					bt_diver[i].on_callback(i); // переиндексацию сделать
				break;
			case true:  // button push-down
				if (bt_diver[i].off_callback != NULL)
					bt_diver[i].off_callback(i);
				break;
			}
		}
	}
}
