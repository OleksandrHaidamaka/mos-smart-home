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
#define LED_PIN     (D4)

#define led_off()    pin_write(LED_PIN, true)
#define led_on()     pin_write(LED_PIN, false)

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
				time = 20;
				break;
			}
			led_on();
			time = 2;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_off();
			time = 2;
			state--;
		}
		break;
	}
}
