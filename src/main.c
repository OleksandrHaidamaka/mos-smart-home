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
	/* Driver handlers */
	drv_led_handler();
	drv_switch_handler();
	drv_button_handler();
	drv_mqtt_handler();

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
static void wifi_handler(enum mgos_wifi_status event, void *data)
{
	(void) data;
	switch ((int) event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		wifi_ip_acquired = true;
		get_cfg()->mqtt.enable = true;
		if (timer_id == 0)
			timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
		break;
	case MGOS_WIFI_CONNECTED:
		wifi_ip_acquired = false;
		drv_mqtt_connack = false;
		get_cfg()->mqtt.enable = false;
		if (timer_id)
		{
			mgos_clear_timer(timer_id);
			timer_id = 0;
			drv_led_pwm(gl_drv_led_pwm);
		}
		break;
	case MGOS_WIFI_DISCONNECTED:
		wifi_ip_acquired = false;
		drv_mqtt_connack = false;
		get_cfg()->mqtt.enable = false;
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
	drv_led_init();
	drv_switch_init();
	drv_button_init();

	/* IoT init */
	iot_relay_init();
	iot_switch_init();
	iot_switch_relay_init();
	iot_button_init();
	iot_button_relay_init();

	timer_id = mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
	mgos_wifi_add_on_change_cb(wifi_handler, 0);
	mgos_mqtt_add_global_handler(drv_mqtt_callback, NULL);

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
