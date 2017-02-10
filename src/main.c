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

#define  NUM_NODES               get_cfg()->settings.num_nodes
#define  LIGHT_PIN(n)   ((int *)&get_cfg()->settings.light.pin_0)[n]
#define  LIGHT_ID(n)    ((int *)&get_cfg()->settings.light.id_0)[n]
#define  SWITCH_PIN(n)  ((int *)&get_cfg()->settings.sw.pin_0)[n]

/*******************************************************************************
 *** CUSTOM DEFENITIONS
 ******************************************************************************/
#define  DEBOUNCE_TIME  (0.05)  // seconds
#define  MQTT_DELAY     (1000)  // sending timeout

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define pin_input_up(pin)    { mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_INPUT);  \
                               mgos_gpio_set_pull(pin, MGOS_GPIO_PULL_UP); }
#define pin_output(pin)        mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT);
#define pin_write(pin, state)  mgos_gpio_write(pin, state)
#define pin_read(pin)          mgos_gpio_read(pin)
#define led_off()              pin_write(LED_PIN, true)
#define led_on()               pin_write(LED_PIN, false)

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
switch_state_t* switch_state;
//void(*mqtt_handler) (void) = NULL;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
const char* topic_request = "home/request";
const char* topic_response = "home/response";

static void timer_handler();
static void wifi_handler(enum mgos_wifi_status event, void *data);

void mqtt_pub(const char *cmd, ...);

static void welcome_str()
{
	printf("\n\n*** Welcome to <mos-hall> project ***\n");
	printf("*** Compile time: %s **********\n\n", __TIME__);
}

static void init_switchs()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		switch_state[i].s_new = switch_state[i].s_old = pin_read(SWITCH_PIN(i));
		switch_state[i].update = true;
		pin_write(LIGHT_PIN(i), switch_state[i].s_old);
		printf("%s(): switch %d = %d\r\n", __func__, i, switch_state[i].s_old);
	}
}

static void low_level_init()
{
	switch_state = calloc(NUM_NODES, sizeof(switch_state_t));

	welcome_str();

	pin_output(LED_PIN);
	led_off();
	for (int i = 0; i < NUM_NODES; i++)
	{
		pin_input_up(SWITCH_PIN(i));
		pin_output(LIGHT_PIN(i));
	}

	init_switchs();
	mgos_set_timer(50, true, timer_handler, NULL);
	mgos_wifi_add_on_change_cb(wifi_handler, 0);

//	mqtt_handler = mqtt_pub_init_state;
}

static void button_handler()
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		bool state = pin_read(SWITCH_PIN(i));

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
				pin_write(LIGHT_PIN(i), switch_state[i].s_old);
				printf("%s(): switch %d = %d\n", __func__, i,
						switch_state[i].s_old);
				mqtt_pub("{light: %d, state: %d}", LIGHT_ID(i),
						switch_state[i].s_old);
				switch_state[i].update = 1;
			}
		}
	}
}

static void blink_driver()
{
	static int time = 0;
	static int state = 0;
	static int count = 0;

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
		printf("pub: topic: <error> msg: error\n");
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
	printf("pub: topic: <%s> msg: %s\n", topic_response, msg);
}

void mqtt_light_id(int id, bool state)
{
	for (int i = 0; i < NUM_NODES; i++)
	{
		if (id == LIGHT_ID(i))
		{
			pin_write(LIGHT_PIN(i), state);
			printf("id = %d; status: accepted\n", id);
			return;
		}
	}
	printf("id = %d; status: ignored\n", id);
}

static void mqtt_cmd_parcer(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;

	int id, state;

	printf("sub: topic: <%s> ", topic_request);
	if (json_scanf(s->p, s->len, "{light: %d, state: %d}", &id, &state) == 2)
	{
		printf("msg: %.*s\n", (int) s->len, s->p);
		mqtt_light_id(id, (bool) state);
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

	switch (ev)
	{
	case MG_EV_MQTT_CONNACK:
		if (msg->connack_ret_code == 0)
		{
			//blink 3
			mqtt_sub();
		}
		break;
	case MG_EV_MQTT_PUBLISH:
		mqtt_cmd_parcer(msg);
		break;
	}
}

static void wifi_handler(enum mgos_wifi_status event, void *data)
{
	(void) data;
	switch ((int) event)
	{
	case MGOS_WIFI_IP_ACQUIRED:
		//blink 2
		mgos_mqtt_set_global_handler(mqtt_handler, NULL);
		break;
	case MGOS_WIFI_DISCONNECTED:
		//blink 1
		mgos_mqtt_set_global_handler(NULL, NULL);
		break;
	}
}

enum mgos_app_init_result mgos_app_init(void)
{
	low_level_init();
	return 0;
}
