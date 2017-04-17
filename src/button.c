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
//switch_state_t* switch_state;

//------------------------------------------------------------------------------
//static void button_periph()
//{
//	for (int i = 0; i < NUM_NODES; i++)
//	{
//		pin_input_up(SWITCH_PIN(i));
//	}
//}

//------------------------------------------------------------------------------
void button_init()
{
//	switch_state = calloc(NUM_NODES, sizeof(switch_state_t));
//
//	switch_periph();
//	light_periph();
//
//	for (int i = 0; i < NUM_NODES; i++)
//	{
//		switch_state[i].s_new = switch_state[i].s_old = pin_read(SWITCH_PIN(i));
//		switch_state[i].update = true;
//		pin_write(LIGHT_PIN(i), switch_state[i].s_old);
//		printf("%s(): switch %d = %d\r\n", __func__, i, switch_state[i].s_old);
//	}
}

//------------------------------------------------------------------------------
void button_driver()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		if(switch_state[i].update == true)
		{

		}
	}
}
