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
sw_t sw[NUM_SW_IOT] = 															// switch object
{
	{ .in = D2, .mqtt = true },
	{ .in = D3, .mqtt = true },
};

sw_relay_t sw_relay[NUM_SW_RELAY_IOT] = 										// switch_relay object
{
	{{ .in = D2, .out = D5 }, .mqtt = true },
	{{ .in = D3, .out = D6 }, .mqtt = true },
};

bt_t bt[NUM_BT_IOT] = 															// button object
{
	{ .in = D2, .mqtt = true },
	{ .in = D3, .mqtt = true },
};

bt_relay_t bt_relay[NUM_BT_RELAY_IOT] = 										// button_relay object
{
	{{ .in = D2, .out = D5 }, .mqtt = true },
	{{ .in = D3, .out = D6 }, .mqtt = true },
};
