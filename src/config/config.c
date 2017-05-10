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
sw_relay_t sw_relay[NUM_SW_RELAY_IOT] = 										// switch_relay object
{
	{{ .in = D1, .out = D5 }, .mqtt = true },
	{{ .in = D2, .out = D6 }, .mqtt = true }
};

bt_relay_t bt_relay[NUM_BT_RELAY_IOT] = 										// button_relay object
{
	{{ .in = D1, .out = D5 }, .mqtt = true },
	{{ .in = D2, .out = D6 }, .mqtt = true }
};
