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
	void (*bt_handler)(int);
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
void iot_button_relay_mode_task_handler(int i, void (*handler)(int));
void iot_button_relay_task_go_to_normal(int i);
void iot_button_relay_task_sos(int i);
void iot_button_relay_task_panic(int i);

#endif  //__IOT_BT_RELAY_H__
