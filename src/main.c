/*
 @autor:       Alexandr Haidamaka
 @file:        main.c
 @description: Main functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

//------------------------------------------------------------------------------
static void welcome_str()
{
	printf("\n\n*** Welcome to <mos-hall> project ***\n");
	printf("*** Compile time: %s **********\n\n", __TIME__);
}

//------------------------------------------------------------------------------
static void wifi_handler(enum mgos_wifi_status event, void *data)
{
	(void) data;
	switch ((int) event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		blink_mode(BL_WIFI_IP_ACQUIRED);
		break;
	case MGOS_WIFI_DISCONNECTED:
		blink_mode(BL_WIFI_DISCONNECTED);
		break;
	}
}

//------------------------------------------------------------------------------
static void sys_tick()
{
	led_driver();
	switch_driver();
	if (mqtt_callback != NULL)
		mqtt_callback();
}

static void sys_tick_start()
{
	mgos_set_timer(SYS_TICK, true, sys_tick, NULL);
}

//------------------------------------------------------------------------------
static void __low_level_init()
{
	led_init();
	switch_init();
	mgos_wifi_add_on_change_cb(wifi_handler, 0);
	mgos_mqtt_add_global_handler(mqtt_handler, NULL, NULL);
	mgos_set_timer(10000, false, sys_tick_start, NULL);
}

//------------------------------------------------------------------------------
enum mgos_app_init_result mgos_app_init(void)
{
	welcome_str();
	__low_level_init();
	return 0;
}
