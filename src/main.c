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
bool wifi_ip_acquired;

//------------------------------------------------------------------------------
static void welcome_str()
{
	printf("\n\n*** Welcome to <mos-hall> project ***\n");
	printf("*** Compile time: %s **********\n\n", __TIME__);
}

//------------------------------------------------------------------------------
static void sys_tick()
{
	led_driver();
	switch_driver();
	button_driver();
	mqtt_driver();
}

//------------------------------------------------------------------------------
static void wifi_sta_start()
{
	get_cfg()->wifi.sta.enable = true;
	mgos_wifi_setup_sta(&get_cfg()->wifi.sta);
}

//------------------------------------------------------------------------------
static void delay(int ms)
{
	if (ms)
	{
		mgos_ints_disable();
		mgos_msleep(ms);
	}
}

//------------------------------------------------------------------------------
static void wifi_handler(enum mgos_wifi_status event, void *data)
{
	(void) data;
	switch ((int) event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		wifi_ip_acquired = true;
		if (timer_id == 0)
			timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
		break;
	case MGOS_WIFI_CONNECTED:
		wifi_ip_acquired = false;
		if (timer_id)
		{
			mgos_clear_timer(timer_id);
			timer_id = 0;
			led_pwm(gl_led_pwm);
		}
		break;
	case MGOS_WIFI_DISCONNECTED:
		wifi_ip_acquired = false;
		if (timer_id == 0)
			timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
		blink_mode(BL_WIFI_DISCONNECTED);
		break;
	}
}

//------------------------------------------------------------------------------
static void __low_level_init()
{
	led_init();
	switch_driver_init();
	switch_relay_init();
	button_driver_init();
	button_relay_init();
	timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
	mgos_wifi_add_on_change_cb(wifi_handler, 0);
	mgos_mqtt_add_global_handler(mqtt_handler, NULL);

	/* Отложенный старт wifi модуля */
	mgos_set_timer(1000, false, wifi_sta_start, NULL);
}

//------------------------------------------------------------------------------
enum mgos_app_init_result mgos_app_init(void)
{
	delay(100);
	welcome_str();
	__low_level_init();
	return 0;
}
