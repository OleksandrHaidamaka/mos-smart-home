#ifndef __DRV_MQTT_H__
#define __DRV_MQTT_H__

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_mqtt_pub(const char *cmd, ...);
void drv_mqtt_handler(void);
void drv_mqtt_callback(struct mg_connection *c, int ev, void *p, void* user_data);

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern bool drv_mqtt_connack;

#endif  //__DRV_MQTT_H__
