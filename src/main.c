/*
 @autor: Alexandr Haidamaka
 @ver:   1.0
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include <string.h>
#include "common/platform.h"
#include "fw/src/mgos_app.h"
#include "fw/src/mgos_gpio.h"
#include "fw/src/mgos_timers.h"
#include "fw/src/mgos_wifi.h"
#include "fw/src/mgos_mqtt.h"

/*******************************************************************************
 *** COMMON DEFENITIONS
 ******************************************************************************/
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

/*******************************************************************************
 *** CUSTOM DEFENITIONS
 ******************************************************************************/
#define NUM_SWITCHES   (2)     // number of switches
#define DEBOUNCE_TIME  (0.05)  // seconds
#define MQTT_DELAY     (1000)  // sending timeout

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define led_off()  mgos_gpio_write(LED_PIN, true)
#define led_on()   mgos_gpio_write(LED_PIN, false)

/*******************************************************************************
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	double elapsed_time;
	bool s_new;
	bool s_old;
	bool update;
} switch_state_t;

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
mgos_timer_id timer_id = NULL;
const uint8_t switch_pin[NUM_SWITCHES] =
{ D2, D3 }; // connect appropriate pins
const uint8_t light_pin[NUM_SWITCHES] =
{ D5, D6 };  // connect appropriate pins
switch_state_t switch_state[NUM_SWITCHES];
//void(*mqtt_handler) (void) = NULL;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
static void timer_handler();

static void welcome_str()
{
	printf("\n\n*** Welcome to mos Hall project ***\n");
	printf("*** Compile time: %s ********\n\n", __TIME__);
}

static void init_switchs()
{
	welcome_str();
	for (uint8_t i = 0; i < NUM_SWITCHES; i++)
	{
		switch_state[i].s_new = switch_state[i].s_old = mgos_gpio_read(
				switch_pin[i]);
		switch_state[i].update = true;
		mgos_gpio_write(light_pin[i], switch_state[i].s_old);
		printf("%s(): switch %d = %d\r\n", __func__, i, switch_state[i].s_old);
	}
}

static void low_level_init()
{
	mgos_gpio_set_mode(LED_PIN, MGOS_GPIO_MODE_OUTPUT);
	led_off();

	for (uint8_t i = 0; i < NUM_SWITCHES; i++)
	{
		mgos_gpio_set_mode(switch_pin[i], MGOS_GPIO_MODE_INPUT);
		mgos_gpio_set_pull(switch_pin[i], MGOS_GPIO_PULL_UP);
		mgos_gpio_set_mode(light_pin[i], MGOS_GPIO_MODE_OUTPUT);
	}

	init_switchs();
	timer_id = mgos_set_timer(50, true, timer_handler, NULL);
//	mqtt_handler = mqtt_pub_init_state;
}

static void button_handler()
{
	for (uint8_t i = 0; i < NUM_SWITCHES; i++)
	{
		uint8_t state = mgos_gpio_read(switch_pin[i]);

		// If the switch changed, due to noise or pressing:
		if (state != switch_state[i].s_new)
		{
			switch_state[i].s_new = state;
			switch_state[i].elapsed_time = mg_time() + DEBOUNCE_TIME;
		}

		if ((mg_time() > switch_state[i].elapsed_time))
		{
			if (state != switch_state[i].s_old)
			{
				switch_state[i].s_old = state;
				mgos_gpio_write(light_pin[i], switch_state[i].s_old);
				switch_state[i].update = 1;
				printf("%s(): switch %d = %d\n", __func__, i,
						switch_state[i].s_old);
			}
		}
	}
}

static void blink_driver()
{
	static uint32_t time = 0;
	static uint8_t state = 0;
	static uint8_t count = 0;

	if (time != 0)
		time--;

	switch (state)
	{
	case 0:
		if (time == 0)
		{
			if (count == 2)
			{
				led_off();
				count = 0;
				time = 20;
				break;
			}
			led_on();
			time = 2;
			state++;
			count++;
		}
		break;
	case 1:
		if (time == 0)
		{
			led_off();
			time = 2;
			state--;
		}
		break;
	}
}

static void timer_handler()
{
	blink_driver();
	button_handler();
}

#define NUM_TOPICS   (2)

const char topic0[] = "hall/light/switch/first";
const char topic1[] = "hall/light/switch/second";

const char* topics[NUM_TOPICS] =
{ topic0, topic1 };

void mqtt_subscribe(struct mg_connection *c, const char* topic, uint8_t qos)
{
	struct mg_mqtt_topic_expression topic_msg =
	{ topic, qos };
	mg_mqtt_subscribe(c, &topic_msg, 1, 42);
	printf("mqtt: subscribed to \"%s\" topic\n", topic);
}

static int getlen(char* s)
{
	int i;

	for (i = 0; (s[i] != ' ') && (s[i] != '\0'); i++)
	{
		if (i > 128)
		{
			s[0] = '\0';
			return 0;
		}
	}
	s[i] = '\0';
	return i - 1;
}

static void topic_parcer(struct mg_mqtt_message* msg)
{
	char* topic = malloc(msg->topic.len);
	char* payload = malloc(msg->payload.len);

	memset(topic, 0, msg->topic.len);
	memset(payload, 0, msg->payload.len);

	strncpy(topic, msg->topic.p, getlen((char*) msg->topic.p));
	strncpy(payload, msg->payload.p, getlen((char*) msg->payload.p));
	printf("mqtt sub: topic: %s; payload: %s\n", topic, payload);

}

static void mqtt_handler(struct mg_connection *c, int ev, void *p)
{
	struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

	printf("Mqtt event [%d]: err = %d\n", ev, msg->connack_ret_code);

	if (ev == MG_EV_MQTT_CONNACK)
	{
		//blink 4
		if (msg->connack_ret_code == 0)
		{
			mqtt_subscribe(c, topics[0], 0);
			mqtt_subscribe(c, topics[1], 0);
		}
	}
	else if (ev == MG_EV_MQTT_PUBLISH)
	{
		topic_parcer(msg);
	}
}

static void wifi_handler(enum mgos_wifi_status event, void *data)
{

	(void) data;
	switch (event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		//blink 3
		mgos_mqtt_set_global_handler(mqtt_handler, NULL);
		break;
	case MGOS_WIFI_CONNECTED:
		//blink 2
		break;
	case MGOS_WIFI_DISCONNECTED:
		//blink 1
		break;
	}
}

enum mgos_app_init_result mgos_app_init(void)
{
	low_level_init();
	mgos_wifi_add_on_change_cb(wifi_handler, 0);

	return MGOS_APP_INIT_SUCCESS;
}
