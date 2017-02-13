#ifndef __LED_H__
#define __LED_H__

/*******************************************************************************
 *** ENUM
 ******************************************************************************/
enum bl_mode
{
	BL_WIFI_DISCONNECTED,
	BL_WIFI_IP_ACQUIRED,
	BL_MQTT_CONNECTED,
	BL_MQTT_SUB_MSG_ERR,
	BL_MQTT_SUB_MSG_OK,
	BL_MQTT_PUB_MSG
};

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void led_init();
void led_driver();
void blink_mode(enum bl_mode);

#endif  //__LED_H__
