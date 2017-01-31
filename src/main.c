/*
 @autor: Alexandr Haidamaka
 @date:  16.01.2017
 */

/******************************
 *** INCLUDES
 ******************************/
#include "common/platform.h"
#include "fw/src/mgos_app.h"
#include "fw/src/mgos_gpio.h"
#include "fw/src/mgos_timers.h"
#include "fw/src/mgos_wifi.h"
//#include "fw/src/mgos_sys_config.h"

/******************************
 *** COMMON DEFENITIONS
 ******************************/
#define __DEBUG

#ifndef __DEBUG
#define  printf()
#endif

#define  D0         (16)
#define  D1         (5)
#define  D2         (4)
#define  D3         (0)
#define  D4         (2)
#define  D5         (14)
#define  D6         (12)
#define  D7         (13)
#define  D8         (15)
#define  D9         (3)
#define  D10        (1)

#define  LED_PIN    (D4)

/******************************
 *** CUSTOM DEFENITIONS
 ******************************/
#define NUM_SWITCHES   (2)     // number of switches
#define DEBOUNCE_TIME  (0.05)  // seconds
#define MQTT_DELAY     (1000)  // sending timeout

/******************************
 *** TYPEDEF
 ******************************/
typedef struct {
	double elapsed_time;
	bool s_new;
	bool s_old;
	bool update;
} switch_state_t;

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
mgos_timer_id timer_id = NULL;
const uint8_t switch_pin[NUM_SWITCHES] = { D2, D3 }; // connect appropriate pins
const uint8_t light_pin[NUM_SWITCHES] = { D5, D6 };  // connect appropriate pins
switch_state_t switch_state[NUM_SWITCHES];
//void(*mqtt_handler) (void) = NULL;

/******************************
 *** PROTOTYPES
 ******************************/
static void timer_handler();

static void welcome_str() {
	printf("\n\n*** Welcome to mos Hall project ***\n");
	printf("*** Compile time: %s ********\n\n", __TIME__);
}

static void init_switchs() {
	welcome_str();
	for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
		switch_state[i].s_new = switch_state[i].s_old = mgos_gpio_read(
				switch_pin[i]);
		switch_state[i].update = true;
		mgos_gpio_write(light_pin[i], switch_state[i].s_old);
		printf("%s(): switch %d = %d\r\n", __func__, i, switch_state[i].s_old);
	}
}

static void low_level_init() {
	mgos_gpio_set_mode(LED_PIN, MGOS_GPIO_MODE_OUTPUT);
	mgos_gpio_write(LED_PIN, false);

	for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
		mgos_gpio_set_mode(switch_pin[i], MGOS_GPIO_MODE_INPUT);
		mgos_gpio_set_pull(switch_pin[i], MGOS_GPIO_PULL_UP);
		mgos_gpio_set_mode(light_pin[i], MGOS_GPIO_MODE_OUTPUT);
	}

	init_switchs();
	timer_id = mgos_set_timer(25, true, timer_handler, NULL);
//	mqtt_handler = mqtt_pub_init_state;
}

static void button_handler() {
	for (uint8_t i = 0; i < NUM_SWITCHES; i++) {
		uint8_t state = mgos_gpio_read(switch_pin[i]);

		// If the switch changed, due to noise or pressing:
		if (state != switch_state[i].s_new) {
			switch_state[i].s_new = state;
			switch_state[i].elapsed_time = mg_time() + DEBOUNCE_TIME;
		}

		if ((mg_time() > switch_state[i].elapsed_time)) {
			if (state != switch_state[i].s_old) {
				switch_state[i].s_old = state;
				mgos_gpio_write(light_pin[i], switch_state[i].s_old);
				switch_state[i].update = 1;
				printf("%s(): switch %d = %d\n", __func__, i,
						switch_state[i].s_old);
			}
		}
	}
}

static void timer_handler() {
	button_handler();
}

static void long_timer_handler() {
	mgos_gpio_toggle(LED_PIN);
}

static void on_wifi_event(enum mgos_wifi_status event, void *data) {
	(void) data;
	switch (event) {
	case MGOS_WIFI_IP_ACQUIRED:
		break;
	case MGOS_WIFI_CONNECTED:
		break;
	case MGOS_WIFI_DISCONNECTED:
		break;
	}
}

enum mgos_app_init_result mgos_app_init(void) {

	mgos_set_timer(250, false, low_level_init, NULL);
	mgos_set_timer(2500, true, long_timer_handler, NULL);
	mgos_wifi_add_on_change_cb(on_wifi_event, 0);

	return MGOS_APP_INIT_SUCCESS;
}
