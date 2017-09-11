/*
 @autor:       Alexandr Haidamaka
 @file:        config.c
 @description: configuration file
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
iot_relay_t iot_relay[NUM_IOT_RELAY] = 					// relay object
{
//	{ .out = D5, .mqtt = NONE },
//	{ .out = D6, .mqtt = NONE },
};

iot_sw_t iot_sw[NUM_IOT_SW] = 							// switch object
{
//	{ .in = D2, .mqtt = NONE },
//	{ .in = D3, .mqtt = NONE },
};

iot_sw_relay_t iot_sw_relay[NUM_IOT_SW_RELAY] = 		// switch_relay object
{
//	{{ .in = D2, .out = D5 }, .mqtt = NONE },
//	{{ .in = D3, .out = D6 }, .mqtt = NONE },
};

iot_bt_t iot_bt[NUM_IOT_BT] = 							// button object
{
//	{ .in = D2, .mqtt = NONE },
//	{ .in = D3, .mqtt = NONE },
};

iot_bt_relay_t iot_bt_relay[NUM_IOT_BT_RELAY] = 		// button_relay object
{
	{{ .in = D2, .out = D5 }, .mqtt = NONE },
	{{ .in = D1, .out = D6 }, .mqtt = NONE },
};
