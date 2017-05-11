#ifndef __MQTT_H__
#define __MQTT_H__

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void mqtt_pub(const char *cmd, ...);
void mqtt_driver_handler();
void mqtt_handler(struct mg_connection *c, int ev, void *p, void* user_data);

char* bool_to_str_state(bool state);

#endif  //__MQTT_H__
