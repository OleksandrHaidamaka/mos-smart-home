/*
 @autor:       Alexandr Haidamaka
 @file:        mqtt.c
 @description: MQTT driver functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "../main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  MQTT_SEND_TIMEOUT  (250/SYS_TICK)  // time [ms]

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    get_cfg()->mqtt.pub
#define  SUB_TOPIC()    get_cfg()->mqtt.sub
#define  MQTT_ACK()     get_cfg()->mqtt.ack

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
bool mqtt_connack = false;

//------------------------------------------------------------------------------
static void mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	struct mg_mqtt_topic_expression topic_exp =
	{ SUB_TOPIC(), 0 };
	mg_mqtt_subscribe(c, &topic_exp, 1, 42);
	printf("%s(%s)\n", __func__, SUB_TOPIC());
}

//------------------------------------------------------------------------------
void mqtt_pub(const char *cmd, ...)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	char msg[128];
	static uint16_t message_id;
	struct json_out jmo = JSON_OUT_BUF(msg, sizeof(msg));
	va_list ap;
	int n;
	va_start(ap, cmd);
	n = json_vprintf(&jmo, cmd, ap);
	va_end(ap);
	mg_mqtt_publish(c, PUB_TOPIC(), message_id++, MG_MQTT_QOS(0), msg, n);
	blink_mode(BL_MQTT_PUB_MSG);
	printf("%s(%s msg: %s)\n", __func__, PUB_TOPIC(), msg);
}

//------------------------------------------------------------------------------
static char* bool_to_str_state(bool state)
{
	return (state == true ? "ON" : "OFF");
}

//------------------------------------------------------------------------------
static bool str_to_bool_state(char* state)
{
	return (strcmp(state, "ON") ? false : true);
}

//------------------------------------------------------------------------------
static void mqtt_relay_action(int i, bool state)
{
	if (i < NUM_RELAY_IOT)
	{
		pin_write(relay[i].out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_sw_relay_action(int i, bool state)
{
	if (i < NUM_SW_RELAY_IOT)
	{
		pin_write(sw_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_bt_relay_action(int i, bool state)
{
	if (i < NUM_BT_RELAY_IOT)
	{
		pin_write(bt_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_get_status()
{
	int i;

	for (i = 0; i < NUM_RELAY_IOT; i++)
		relay[i].mqtt = true;

	for (i = 0; i < NUM_SW_IOT; i++)
		sw[i].mqtt = true;

	for (i = 0; i < NUM_SW_RELAY_IOT; i++)
		sw_relay[i].mqtt = true;

	for (i = 0; i < NUM_BT_IOT; i++)
		bt[i].mqtt = true;

	for (i = 0; i < NUM_BT_RELAY_IOT; i++)
		bt_relay[i].mqtt = true;

	gl_led_mqtt = true;
}

//------------------------------------------------------------------------------
void mqtt_driver_handler()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	static int time = 0;
	int i;

	if ((c == NULL) || (mqtt_connack == false))
	{
		mqtt_connack = false;
		time = 0;
		if (wifi_ip_acquired == true)
			blink_mode(BL_WIFI_IP_ACQUIRED);
		return;
	}

	if (time++ < MQTT_SEND_TIMEOUT)
		return;
	time = 0;

	for (i = 0; i < NUM_RELAY_IOT; i++)
	{
		if (relay[i].mqtt == true)
		{
			relay[i].mqtt = false;
			mqtt_pub("{relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(relay[i].out)));
			return;
		}
	}

	for (i = 0; i < NUM_SW_IOT; i++)
	{
		if (sw[i].mqtt == true)
		{
			sw[i].mqtt = false;
			mqtt_pub("{sw: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(sw[i].in)));
			return;
		}
	}

	for (i = 0; i < NUM_SW_RELAY_IOT; i++)
	{
		if (sw_relay[i].mqtt == true)
		{
			sw_relay[i].mqtt = false;
			mqtt_pub("{sw_relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(sw_relay[i].pin.in)));
			return;
		}
	}

	for (i = 0; i < NUM_BT_IOT; i++)
	{
		if (bt[i].mqtt == true)
		{
			bt[i].mqtt = false;
			mqtt_pub("{bt: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(bt[i].in)));
			return;
		}
	}

	for (i = 0; i < NUM_BT_RELAY_IOT; i++)
	{
		if (bt_relay[i].mqtt == true)
		{
			bt_relay[i].mqtt = false;
			mqtt_pub("{bt_relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(bt_relay[i].pin.out)));
			return;
		}
	}

	if (gl_led_mqtt == true)
	{
		gl_led_mqtt = false;
		mqtt_pub("{led: %d}", gl_led_pwm);
	}
}

//------------------------------------------------------------------------------
static void mqtt_parcer_msg(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;
	int i;
	bool err = false;
	char* state;

	printf("%s(%s msg: %.*s)\n", __func__, SUB_TOPIC(), (int) s->len, s->p);

	/* parsing commands */
	if (json_scanf(s->p, s->len, "{relay: %d, state: %Q}", &i, &state) == 2)
	{
		mqtt_relay_action(i, str_to_bool_state(state));
	}
	else if (json_scanf(s->p, s->len, "{sw_relay: %d, state: %Q}", &i, &state)
			== 2)
	{
		mqtt_sw_relay_action(i, str_to_bool_state(state));
	}
	else if (json_scanf(s->p, s->len, "{bt_relay: %d, state: %Q}", &i, &state)
			== 2)
	{
		mqtt_bt_relay_action(i, str_to_bool_state(state));
	}
	else if (json_scanf(s->p, s->len, "{led: %d}", &i) == 1)
	{
		gl_led_pwm = abs(i);
		if (gl_led_pwm > 100)
			gl_led_pwm = 100;
	}
	else if (strncmp(s->p, "get status", sizeof("get status") - 1) == 0)
	{
		mqtt_get_status();
	}
	else
	{
		err = true;
	}

	if (err == false)
	{
		blink_mode(BL_MQTT_SUB_MSG_OK);
		if (MQTT_ACK() == true)
			mqtt_pub("%.*s\n", (int) s->len, s->p);
	}
	else
	{
		blink_mode(BL_MQTT_SUB_MSG_ERR);
	}
}

//------------------------------------------------------------------------------
void mqtt_handler(struct mg_connection *c, int ev, void *p, void* user_data)
{
	(void) c;
	(void) user_data;
	struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

	switch (ev)
	{
	case MG_EV_MQTT_CONNACK:
		if (msg->connack_ret_code == 0)
		{
			if (PUB_TOPIC() != NULL && SUB_TOPIC() != NULL)
			{
				mqtt_connack = true;
				mqtt_sub();
				blink_mode(BL_MQTT_CONNECTED);
			}
			else
				printf("run 'mos config-set mqtt.sub=... mqtt.pub=...'\n");
		}
		break;
	case MG_EV_MQTT_PUBLISH:
		mqtt_parcer_msg(msg);
		break;
	}
}
