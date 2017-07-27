/*
 @autor:       Alexandr Haidamaka
 @file:        main.c
 @description: Main functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
mgos_timer_id timer_id;

//------------------------------------------------------------------------------
static void welcome_str()
{
	printf("\n\n*** Welcome to <mos-hall> project ***\n");
	printf("*** Compile time: %s **********\n\n", __TIME__);
}

//------------------------------------------------------------------------------
static void sys_tick()
{
	/* Driver handlers */
	drv_switch_handler();
	drv_button_handler();

	if (drv_led.handler != NULL)
		drv_led.handler();

	if (drv_mqtt.handler != NULL)
		drv_mqtt.handler();

	/* IoT handlers */
	iot_button_relay_handler();
}

//------------------------------------------------------------------------------
static void wifi_sta_start()
{
	get_cfg()->wifi.sta.enable = true;
	mgos_wifi_setup_sta(&get_cfg()->wifi.sta);
}

//------------------------------------------------------------------------------
void delay(int ms)
{
	if (ms)
	{
		mgos_ints_disable();
		mgos_msleep(ms);
	}
}

//------------------------------------------------------------------------------
static void drv_wifi_callback(enum mgos_wifi_status event, void *data)
{
	(void) data;
	switch ((int) event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		get_cfg()->mqtt.enable = true;
		if (timer_id == 0)
			timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
		drv_led_blink_mode(BL_WIFI_IP_ACQUIRED);
		break;
	case MGOS_WIFI_CONNECTED:
		if (timer_id)
		{
			mgos_clear_timer(timer_id);
			timer_id = 0;
			drv_led_init();
		}
		break;
	case MGOS_WIFI_DISCONNECTED:
		get_cfg()->mqtt.enable = false;
		drv_mqtt.handler = NULL;
		if (timer_id == 0)
			timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
		drv_led_blink_mode(BL_WIFI_DISCONNECTED);
		break;
	}
}

//------------------------------------------------------------------------------
static void __low_level_init()
{
	/* Driver init */
	hal_tim_init();
	drv_led_init();
	drv_dimmer_init();
	drv_switch_init();
	drv_button_init();

	/* IoT init */
	iot_relay_init();
	iot_dimmer_init();
	iot_switch_init();
	iot_switch_relay_init();
	iot_button_init();
	iot_button_relay_init();

	/* регистрация wifi & mqtt callback's */
	mgos_wifi_add_on_change_cb(drv_wifi_callback, NULL);
	mgos_mqtt_add_global_handler(drv_mqtt_callback, NULL);

	/* старт sys_tick таймера */
	mgos_set_timer(SYS_TICK, true, sys_tick, NULL);

	/* oтложенный старт wifi модуля */
	mgos_set_timer(1000, false, wifi_sta_start, NULL);
}

//------------------------------------------------------------------------------
enum mgos_app_init_result mgos_app_init(void)
{
	delay(0);
	welcome_str();
	__low_level_init();
	return 0;
}
