#ifndef __IOT_BT_RELAY_H__
#define __IOT_BT_RELAY_H__

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "iot.h"

/*******************************************************************************
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	drv_in_out_t pin;
	bool mqtt;
	iot_mode_t mode;
} iot_bt_relay_t;

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern iot_bt_relay_t iot_bt_relay[NUM_IOT_BT_RELAY];

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void iot_button_relay_init(void);
void iot_button_relay_off_callback(int i);
void iot_button_relay_on_callback(int i);
void iot_button_relay_handler(void);

#endif  //__IOT_BT_RELAY_H__
