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
 *** VARIABLES
 ******************************************************************************/
switch_state_t* switch_state;

//------------------------------------------------------------------------------
static void switch_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_input_up(SWITCH_PIN(i));
	}
}

//------------------------------------------------------------------------------
static void light_periph()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_output(LIGHT_PIN(i));
	}
}

//------------------------------------------------------------------------------
void switch_init()
{
	switch_state = calloc(NUM_NODES, sizeof(switch_state_t));

	switch_periph();
	light_periph();

	for (int i = 0; i < NUM_NODES; i++)
	{
		switch_state[i].s_new = switch_state[i].s_old = pin_read(SWITCH_PIN(i));
		switch_state[i].update = true;
		pin_write(LIGHT_PIN(i), switch_state[i].s_old);
		printf("%s(): switch %d = %d\r\n", __func__, i, switch_state[i].s_old);
	}
}

//------------------------------------------------------------------------------
void switch_driver()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		bool state = pin_read(SWITCH_PIN(i));

		// If the switch changed, due to noise or pressing:
		if (state != switch_state[i].s_new)
		{
			switch_state[i].s_new = state;
			switch_state[i].elapsed_time = mg_time() + DEBOUNCE_TIME;
		}

		if ((mg_time() > switch_state[i].elapsed_time))
		{
			if (state != switch_state[i].s_old)
			{
				switch_state[i].s_old = state;
				pin_write(LIGHT_PIN(i), switch_state[i].s_old);
				printf("%s(): switch %d = %d\n", __func__, i,
						switch_state[i].s_old);
				mqtt_pub("{light_id: %d, state: %d}", LIGHT_ID(i),
						switch_state[i].s_old);
				switch_state[i].update = 1;
			}
		}
	}
}

