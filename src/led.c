/*
 @autor:       Alexandr Haidamaka
 @file:        led.c
 @description: Led driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define BLYNK_LONG_TIME    (1500/SYS_TICK)  // time [ms]
#define BLYNK_SHORT_TIME   (250/SYS_TICK)   // time [ms]
#define LED_PIN            (D4)
#define led_off()          pin_write(LED_PIN, true)
#define led_on()           pin_write(LED_PIN, false)

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
int gl_blink_mode = 1;

//------------------------------------------------------------------------------
static void led_periph()
{
	pin_output(LED_PIN);
}

//------------------------------------------------------------------------------
void led_init()
{
	led_periph();
	led_off();
}

void blink_mode(int mode)
{
	gl_blink_mode = mode;
}

//------------------------------------------------------------------------------
void led_driver()
{
	static int time = 0;
	static int state = 0;
	static int count = 0;

	if (time != 0)
		time--;

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count == gl_blink_mode)
			{
				led_off();
				count = 0;
				time = BLYNK_LONG_TIME;
				break;
			}
			led_on();
			time = BLYNK_SHORT_TIME;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_off();
			time = BLYNK_SHORT_TIME;
			state--;
		}
		break;
	}
}
