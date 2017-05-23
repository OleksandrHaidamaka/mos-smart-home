/*
 @autor:       Alexandr Haidamaka
 @file:        iot_bt.c
 @description: iot button functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_button_init(void)
{
	for (int i = 0; i < NUM_IOT_BT; i++)
	{
		pin_input(iot_bt[i].in, MGOS_GPIO_PULL_UP);
	}
}

//------------------------------------------------------------------------------
void iot_button_on_callback(int i)
{
	iot_bt[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
