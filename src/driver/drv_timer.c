/*
 @autor:       Alexandr Haidamaka
 @file:        drv_timer.c
 @description: driver timer functionality
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

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_tim_callback(void *arg);

//------------------------------------------------------------------------------
void drv_tim_init(void)
{
	ETS_FRC_TIMER1_INTR_ATTACH(drv_tim_callback, NULL);
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
IRAM NOINSTR void drv_tim_callback(void *arg)
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

	RTC_CLR_REG_MASK(FRC1_INT_ADDRESS, FRC1_INT_CLR_MASK);
}
