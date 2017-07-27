/*
 @autor:       Alexandr Haidamaka
 @file:        hal_timer.c
 @description: hal timer functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define TIM_PRESCALER_16     (4)
#define TIM_RELOAD_VALUE_80  (500 - 8) // T=(V/5) = 500/5 = 100 [us]
#define TIM_FRC1_ENABLE      (1 << 7)
#define TIM_DEBUG            (0) // D7

/*******************************************************************************
 *** STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int hal_pwm[NUM_DRV_DIMMER];
bool zero_detected = false;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void hal_tim_callback(void *arg);

//------------------------------------------------------------------------------
void hal_tim_init(void)
{
	if (NUM_DRV_DIMMER == 0)
		return;
	ETS_FRC_TIMER1_INTR_ATTACH(hal_tim_callback, NULL);
	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, TIM_RELOAD_VALUE_80);
	RTC_REG_WRITE(FRC1_CTRL_ADDRESS, TIM_PRESCALER_16 | TIM_FRC1_ENABLE);
	TM1_EDGE_INT_ENABLE();
	ETS_FRC1_INTR_ENABLE();

#if TIM_DEBUG
	mgos_gpio_set_mode(D7, MGOS_GPIO_MODE_OUTPUT);
#endif
}

//------------------------------------------------------------------------------
IRAM NOINSTR void hal_tim_callback(void *arg)
{
	RTC_REG_WRITE(FRC1_LOAD_ADDRESS, TIM_RELOAD_VALUE_80);
	(void) arg;

#if TIM_DEBUG
	{
		v = GPIO_REG_READ(GPIO_OUT_ADDRESS);
		v ^= 1 << D7;
		GPIO_REG_WRITE(GPIO_OUT_ADDRESS, v);
	}
#endif

	pin_write(D2, false);

	if (zero_detected)
	{
		for (int i = 0; i < NUM_DRV_DIMMER; i++)
		{
			if (hal_pwm[i] != 0)
			{
				if (--hal_pwm[i] == 0)
				{
					pin_write(D2, true);
					zero_detected = false;
					// дернуть ножкой
				}
			}
		}
	}

	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
}
