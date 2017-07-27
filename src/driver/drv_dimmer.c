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

//------------------------------------------------------------------------------
void drv_dimmer_zero_handler(int pin, void* arg)
{
//	printf("zero was detected\n");

	for (int i = 0; i < NUM_DRV_DIMMER; i++)
		hal_pwm[i] = drv_dimmer.pwm[i];

	zero_detected = true;
	(void) pin;
	(void) arg;
}

//------------------------------------------------------------------------------
void drv_dimmer_init(void)
{
	if (NUM_DRV_DIMMER)
	{
		drv_dimmer.pin_zero = iot_dimmer.pin_zero;
		mgos_gpio_set_int_handler(drv_dimmer.pin_zero, MGOS_GPIO_INT_EDGE_POS,
				drv_dimmer_zero_handler, NULL);
		mgos_gpio_enable_int(drv_dimmer.pin_zero);
	}

	for (int i = 0; i < NUM_DRV_DIMMER; i++)
		drv_dimmer.pwm[i] = 85;
}

//------------------------------------------------------------------------------
int drv_dimmer_get_phase(int i)
{
	return drv_dimmer.pwm[i];
}

//------------------------------------------------------------------------------
void drv_dimmer_set_phase(int i, int value)
{
	drv_dimmer.pwm[i] = value;
}
