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
#define  DEBOUNCE_TIME  (0.05)  // time [s]

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
switcher_t* button;

//------------------------------------------------------------------------------
static void button_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_input_up(SWITCH_PIN(i));
	}
}

void button_relay_on_callback(int i)
{
	bool state = !pin_read(LIGHT_PIN(i));
	pin_write(LIGHT_PIN(i), state);
	mqtt_pub("{light: %d, state: %Q}", i, bool_to_str_state(state));
}

//------------------------------------------------------------------------------
static void light_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_output(LIGHT_PIN(i));
	}
}

void button_relay_init(void)
{
	light_periph();

	for (int i = 0; i < NUM_NODES; i++)
	{
		button[i].on_callback = button_relay_on_callback;
		button[i].off_callback = NULL;
		button[i].s_new = switch_state[i].s_old = pin_read(SWITCH_PIN(i));
		pin_write(LIGHT_PIN(i), 0);
		printf("%s(): button %d = %d\r\n", __func__, i, button[i].s_old);
	}
}

//------------------------------------------------------------------------------
void button_init()
{
	button = calloc(NUM_NODES, sizeof(switcher_t));
	button_periph();
	button_relay_init();
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		bool state = pin_read(SWITCH_PIN(i));

		// If the switch changed, due to noise or pressing:
		if (state != button[i].s_new)
		{
			button[i].s_new = state;
			button[i].elapsed_time = mg_time() + DEBOUNCE_TIME;
		}

		if ((mg_time() > button[i].elapsed_time))
		{
			if (state != button[i].s_old)
			{
				button[i].s_old = state;

				switch (state)
				{
				case false: // button push-up
					if (button[i].on_callback != NULL)
						button[i].on_callback(i);
					printf("%s(): button %d = click\n", __func__, i);
					break;
				case true:  // button push-down
					if (button[i].off_callback != NULL)
						button[i].off_callback(i);
					break;
				}
			}
		}
	}
}
