#ifndef __CONFIG_H__
#define __CONFIG_H__

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  NUM_RELAY_IOT         (0)  // количество relay
#define  NUM_SW_IOT            (0)  // количество sw
#define  NUM_SW_RELAY_IOT      (0)  // количество sw-relay
#define  NUM_BT_IOT            (0)  // количество bt
#define  NUM_BT_RELAY_IOT      (2)  // количество bt-relay

/*******************************************************************************
 *** COMMON TYPEDEF
 ******************************************************************************/
typedef struct
{
	int in;
	int out;
} in_out_t;

typedef struct
{
	in_out_t pin_map;
	bool mqtt_update;
} bt_relay_t;

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
bt_relay_t bt_relay[NUM_BT_RELAY_IOT];

#endif  // __CONFIG_H__
