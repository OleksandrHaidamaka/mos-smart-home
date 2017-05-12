/*
 @autor:       Alexandr Haidamaka
 @file:        bt.c
 @description: button iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void button_init(void)
{
	for (int i = 0; i < NUM_BT_IOT; i++)
	{
		pin_input(bt[i].in, MGOS_GPIO_PULL_UP);
	}
}

//------------------------------------------------------------------------------
void button_on_callback(int i)
{
	bt[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
