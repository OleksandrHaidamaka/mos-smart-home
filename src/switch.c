/*
 @autor:       Alexandr Haidamaka
 @file:        switch.c
 @description: Switch driver functionality
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
input_t* sw;
sw_relay_t* sw_relay; // switch_relay object

//------------------------------------------------------------------------------
static void switch_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_input_up(SWITCH_PIN(i));
	}
}

static void switch_relay_off_callback(int i);
static void switch_relay_on_callback(int i);

//------------------------------------------------------------------------------
static void light_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_output(LIGHT_PIN(i));
	}
}

//------------------------------------------------------------------------------
void switch_relay_init(void)
{
	light_periph();

	for (int i = 0; i < NUM_NODES; i++)
	{
		sw_relay[i].mqtt_update = true;
		sw[i].off_callback = switch_relay_off_callback;
		sw[i].on_callback = switch_relay_on_callback;
		sw[i].state = pin_read(SWITCH_PIN(i));
		pin_write(LIGHT_PIN(i), sw[i].state);
	}
}

//------------------------------------------------------------------------------
void switch_init()
{
	sw = calloc(NUM_NODES, sizeof(input_t));
	sw_relay = calloc(NUM_NODES, sizeof(sw_relay_t));
	switch_periph();
	switch_relay_init();
}

//------------------------------------------------------------------------------
static void switch_relay_off_callback(int i)
{
	pin_write(LIGHT_PIN(i), true);
	sw_relay[i].mqtt_update = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
static void switch_relay_on_callback(int i)
{
	pin_write(LIGHT_PIN(i), false);
	sw_relay[i].mqtt_update = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void switch_driver()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		bool state = pin_read(SWITCH_PIN(i));

		if (state != sw[i].state)
		{
			sw[i].state = state;

			switch (state)
			{
			case false: // switch on
				if (sw[i].on_callback != NULL)
					sw[i].on_callback(i);
				break;
			case true:  // switch off
				if (sw[i].off_callback != NULL)
					sw[i].off_callback(i);
				break;
			}
		}
	}
}
