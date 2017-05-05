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

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
input_t* bt;
bt_relay_t* bt_relay; // button_relay object

//------------------------------------------------------------------------------
static void button_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_input_up(SWITCH_PIN(i));
	}
}

static void button_relay_on_callback(int i);

//------------------------------------------------------------------------------
static void light_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_output(LIGHT_PIN(i));
	}
}

//------------------------------------------------------------------------------
void button_relay_init(void)
{
	light_periph();

	for (int i = 0; i < NUM_NODES; i++)
	{
		bt_relay[i].mqtt_update = true;
		bt[i].on_callback = button_relay_on_callback;
		bt[i].off_callback = NULL;
		bt[i].state = pin_read(SWITCH_PIN(i));
		pin_write(LIGHT_PIN(i), true);
	}
}

//------------------------------------------------------------------------------
void button_init()
{
	bt = calloc(NUM_NODES, sizeof(input_t));
	bt_relay = calloc(NUM_NODES, sizeof(bt_relay_t));
	button_periph();
	button_relay_init();
}

//------------------------------------------------------------------------------
void button_relay_on_callback(int i)
{
	pin_write(LIGHT_PIN(i), !pin_read(LIGHT_PIN(i)));
	bt_relay[i].mqtt_update = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		bool state = pin_read(SWITCH_PIN(i));

		if (state != bt[i].state)
		{
			bt[i].state = state;

			switch (state)
			{
			case false: // button push-up
				if (bt[i].on_callback != NULL)
					bt[i].on_callback(i);
				break;
			case true:  // button push-down
				if (bt[i].off_callback != NULL)
					bt[i].off_callback(i);
				break;
			}
		}
	}
}
