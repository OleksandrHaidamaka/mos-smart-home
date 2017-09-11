#ifndef __DRV_MQTT_H__
#define __DRV_MQTT_H__

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "drv.h"
#include "iot.h"

/*******************************************************************************
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	void (*handler)(void);
	int time;
	const char* const mqtt_reason[NUM_MQTT_REASONS];
	const char* const iot_mode[NUM_IOT_MODES];
} drv_mqtt_t;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_mqtt_pub(const char *cmd, ...);
void drv_mqtt_handler(void);
void drv_mqtt_callback(struct mg_connection *c, int ev, void *p,
		void* user_data);

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern drv_mqtt_t drv_mqtt;

#endif  //__DRV_MQTT_H__
