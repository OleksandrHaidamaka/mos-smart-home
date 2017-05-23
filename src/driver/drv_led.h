#ifndef __DRV_LED_H__
#define __DRV_LED_H__

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define LED_PIN            (D4)

/*******************************************************************************
 *** ENUM
 ******************************************************************************/
enum drv_led_blink_mode_t
{
	BL_WIFI_DISCONNECTED,
	BL_WIFI_IP_ACQUIRED,
	BL_MQTT_CONNECTED,
	BL_MQTT_SUB_MSG_ERR,
	BL_MQTT_SUB_MSG_OK,
	BL_MQTT_PUB_MSG
};

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern int gl_drv_led_pwm;
extern bool gl_drv_led_mqtt;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_led_init(void);
void drv_led_handler(void);
void drv_led_blink_mode(enum drv_led_blink_mode_t mode);
void drv_led_pwm(int dim);

#endif  //__DRV_LED_H__
