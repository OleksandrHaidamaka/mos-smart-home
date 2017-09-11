#ifndef __DRV_LED_H__
#define __DRV_LED_H__

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "drv.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define LED_PIN            (D4)
#define led_in()           pin_input(LED_PIN, MGOS_GPIO_PULL_UP)
#define led_out()          pin_output(LED_PIN)
#define led_off_on(state)  pin_write(LED_PIN, !state)

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
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	int time_short;
	int time_long;
	int times;
	bool repeat;
} drv_blink_mode_t;

typedef struct
{
	void (*handler)(void);
	drv_mqtt_reason_e mqtt;
	enum drv_led_blink_mode_t mode_current;
	enum drv_led_blink_mode_t mode_new;
	drv_blink_mode_t mode[];
} drv_led_t;

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern drv_led_t drv_led;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_led_init(void);
void drv_led_blink_mode(enum drv_led_blink_mode_t mode);
void drv_led_handler();

#endif  //__DRV_LED_H__
