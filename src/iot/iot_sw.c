/*
 @autor:       Alexandr Haidamaka
 @file:        iot_sw.c
 @description: iot switch functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

//------------------------------------------------------------------------------
void iot_switch_init(void)
{
	for (int i = 0; i < NUM_IOT_SW; i++)
	{
		pin_input(iot_sw[i].in, MGOS_GPIO_PULL_UP);
	}
}

//------------------------------------------------------------------------------
void iot_switch_off_callback(int i)
{
	iot_sw[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}

//------------------------------------------------------------------------------
void iot_switch_on_callback(int i)
{
	iot_sw[i].mqtt = true;
	printf("%s(%d)\n", __func__, i);
}
