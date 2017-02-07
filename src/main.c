/*
 @autor: Alexandr Haidamaka
 @ver:   1.0
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "common/platform.h"
#include "fw/src/mgos_sys_config.h"
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

#define LIGHT_NUM      get_cfg()->settings.light.num
#define LIGHT_PIN(n)   get_cfg()->settings.light.pin_ ## n
#define LIGHT_ID(n)    get_cfg()->settings.light.id_ ## n

#define SWITCH_NUM     get_cfg()->settings.sw.num
#define SWITCH_PIN(n)  get_cfg()->settings.sw.pin_ ## n
#define SWITCH_ID(n)   get_cfg()->settings.sw.id_ ## n

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
const char* topic_request = "home/request";
const char* topic_response = "home/response";

static void timer_handler();
void mqtt_pub(const char *cmd, ...);

static void welcome_str()
{
	printf("\n\n*** Welcome to <mos-hall> project ***\n");
	printf("*** Compile time: %s **********\n\n", __TIME__);
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
				printf("%s(): switch %d = %d\n", __func__, i,
						switch_state[i].s_old);
				mqtt_pub("{light: %d, state: %d}", i, switch_state[i].s_old);
				switch_state[i].update = 1;
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

/*******************************************************************************
 *** MQTT ENUMS
 ******************************************************************************/
typedef enum
{
	CHANDELIER_COOL = 0, CHANDELIER_WARM, NUM_LIGHT
} home_light_id;

void mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	if (c == NULL)
	{
		printf("sub: topic: <error>\n");
		return;
	}

	struct mg_mqtt_topic_expression topic_exp =
	{ topic_request, 0 };
	mg_mqtt_subscribe(c, &topic_exp, 1, 42);
	printf("sub: topic: <%s>\n", topic_request);
}

void mqtt_pub(const char *cmd, ...)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	if (c == NULL)
	{
		printf("pub: topic <error> msg: error\n");
		return;
	}

	char msg[128];
	static uint16_t message_id;
	struct json_out jmo = JSON_OUT_BUF(msg, sizeof(msg));
	va_list ap;
	int n;
	va_start(ap, cmd);
	n = json_vprintf(&jmo, cmd, ap);
	va_end(ap);
	mg_mqtt_publish(c, topic_response, message_id++, MG_MQTT_QOS(0), msg, n);
	printf("pub: topic <%s> msg: %s\n", topic_response, msg);
}

void mqtt_home_light_io(int num, bool state)
{
	switch (num)
	{
	case CHANDELIER_COOL:
		mgos_gpio_write(light_pin[0], state);
		break;
	case CHANDELIER_WARM:
		mgos_gpio_write(light_pin[1], state);
		break;
	}
}

static void mqtt_cmd_parcer(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;

	int id;
	int param;

	printf("sub: topic: <%s> ", topic_request);
	if (json_scanf(s->p, s->len, "{light: %d, state: %d}", &id, &param) == 2)
	{
		mqtt_home_light_io(id, (bool) param);
		printf("msg: %.*s\n", (int) s->len, s->p);
	}
	else
	{
		printf("msg: <unsupported>\n");
	}
}

static void mqtt_handler(struct mg_connection *c, int ev, void *p)
{
	(void) c;
	struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

	printf("mqtt event [%d]: err = %d\n", ev, msg->connack_ret_code);

	if (ev == MG_EV_MQTT_CONNACK)
	{
		//blink 4
		if (msg->connack_ret_code == 0)
		{
			mqtt_sub();
		}
	}
	else if (ev == MG_EV_MQTT_PUBLISH)
	{
		mqtt_cmd_parcer(msg);
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

	printf("pin = %d\n", LIGHT_PIN(0));
	return MGOS_APP_INIT_SUCCESS;
}
