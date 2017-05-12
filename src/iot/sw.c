/*
 @autor:       Alexandr Haidamaka
 @file:        sw.c
 @description: switch iot functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void switch_init(void)
{
	for (int i = 0; i < NUM_SW_IOT; i++)
	{
		pin_input(sw[i].in, MGOS_GPIO_PULL_UP);
	}
}

//------------------------------------------------------------------------------
void switch_off_callback(int i)
{
	sw[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void switch_on_callback(int i)
{
	sw[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
