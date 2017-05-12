#ifndef __MQTT_H__
#define __MQTT_H__

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void mqtt_pub(const char *cmd, ...);
void mqtt_driver_handler();
void mqtt_handler(struct mg_connection *c, int ev, void *p, void* user_data);

#endif  //__MQTT_H__
