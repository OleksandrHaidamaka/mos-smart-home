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
#define TIM_RELOAD_VALUE_80  (250 - 8) // T=(V/5) = 500/5 = 100 [us]
#define TIM_FRC1_ENABLE      (1 << 7)
#define TIM_DEBUG            (0) // D7

/*******************************************************************************
 *** STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int hal_pwm[NUM_DRV_DIMMER];
int phase_state[NUM_DRV_DIMMER];

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

	static int zero_state = 0;
	static int zero_cnt = 0;

	switch (zero_state)
	{
	case 0:
		if (pin_read(drv_dimmer.pin_zero))
		{
			for (int i = 0; i < NUM_DRV_DIMMER; i++)
			{
				hal_pwm[i] = drv_dimmer.pwm[i];
				phase_state[i] = 1;
			}
			zero_state++;
		}
		break;

	case 1:
		if (++zero_cnt > 100)
		{
			zero_cnt = 0;
			zero_state = 0;
		}
		break;
	}

	switch (phase_state[0])
	{
	case 1:
		phase_state[0] = 4;

		if (hal_pwm[0] == 0)
			pin_write(D2, true);
		else if (hal_pwm[0] == 200)
			pin_write(D2, false);
		else
		{

			hal_pwm[0]--;
			phase_state[0] = 2;
		}
		break;

	case 2:
		if (hal_pwm[0])
			hal_pwm[0]--;
		if (hal_pwm[0] == 0)
		{
			pin_write(D2, true);
			phase_state[0]++;
		}
		break;
	case 3:
		pin_write(D2, false);
		phase_state[0]++;
		break;

	default:
		break;
	}

	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
}
