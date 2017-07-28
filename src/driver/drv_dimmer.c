/*
 @autor:       Alexandr Haidamaka
 @file:        drv_dimmer.c
 @description: driver dimmer functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
drv_dimmer_t drv_dimmer;

const int drv_dimmer_pwm_table[11] =
{ 2*100, 2*87, 2*85, 2*83, 2*81, 2*79, 2*77, 2*75, 2*70, 2*65, 0 };

//------------------------------------------------------------------------------
void drv_dimmer_init(void)
{
	if (NUM_DRV_DIMMER)
		drv_dimmer.pin_zero = iot_dimmer.pin_zero;

	for (int i = 0; i < NUM_DRV_DIMMER; i++)
		drv_dimmer_set_phase(i, 4);
}

//------------------------------------------------------------------------------
int drv_dimmer_get_phase(int i)
{
	return drv_dimmer.pwm[i];
}

//------------------------------------------------------------------------------
void drv_dimmer_set_phase(int i, int value)  // value = 0 .. 10
{
	if (value > 10)
		value = 10;
	else if (value < 0)
		value = 0;

	drv_dimmer.pwm[i] = drv_dimmer_pwm_table[value];
}
