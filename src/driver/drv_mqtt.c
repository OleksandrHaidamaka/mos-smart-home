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
static bool drv_mqtt_connack = false;

//------------------------------------------------------------------------------
static void drv_mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	struct mg_mqtt_topic_expression topic_exp =
	{ SUB_TOPIC(), 0 };
	mg_mqtt_subscribe(c, &topic_exp, 1, 42);
	printf("%s(%s)\n", __func__, SUB_TOPIC());
}

//------------------------------------------------------------------------------
void drv_mqtt_pub(const char *cmd, ...)
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
	drv_led_blink_mode(BL_MQTT_PUB_MSG);
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
	if (i < NUM_IOT_RELAY)
	{
		pin_write(iot_relay[i].out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_sw_relay_action(int i, bool state)
{
	if (i < NUM_IOT_SW_RELAY)
	{
		pin_write(iot_sw_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_bt_relay_action(int i, bool state)
{
	if (i < NUM_IOT_BT_RELAY)
	{
		pin_write(iot_bt_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_get_status()
{
	int i;

	for (i = 0; i < NUM_IOT_RELAY; i++)
		iot_relay[i].mqtt = true;

	for (i = 0; i < NUM_IOT_SW; i++)
		iot_sw[i].mqtt = true;

	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
		iot_sw_relay[i].mqtt = true;

	for (i = 0; i < NUM_IOT_BT; i++)
		iot_bt[i].mqtt = true;

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
		iot_bt_relay[i].mqtt = true;

	gl_drv_led_mqtt = true;
}

//------------------------------------------------------------------------------
void drv_mqtt_handler(void)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	static int time = 0;
	int i;

	if ((c == NULL) || (drv_mqtt_connack == false))
	{
		drv_mqtt_connack = false;
		time = 0;
		if (wifi_ip_acquired == true)
			drv_led_blink_mode(BL_WIFI_IP_ACQUIRED);
		return;
	}

	if (time++ < MQTT_SEND_TIMEOUT)
		return;
	time = 0;

	for (i = 0; i < NUM_IOT_RELAY; i++)
	{
		if (iot_relay[i].mqtt == true)
		{
			iot_relay[i].mqtt = false;
			drv_mqtt_pub("{relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(iot_relay[i].out)));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW; i++)
	{
		if (iot_sw[i].mqtt == true)
		{
			iot_sw[i].mqtt = false;
			drv_mqtt_pub("{sw: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(iot_sw[i].in)));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
	{
		if (iot_sw_relay[i].mqtt == true)
		{
			iot_sw_relay[i].mqtt = false;
			drv_mqtt_pub("{sw_relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(iot_sw_relay[i].pin.in)));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT; i++)
	{
		if (iot_bt[i].mqtt == true)
		{
			iot_bt[i].mqtt = false;
			drv_mqtt_pub("{bt: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(iot_bt[i].in)));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		if (iot_bt_relay[i].mqtt == true)
		{
			iot_bt_relay[i].mqtt = false;
			drv_mqtt_pub("{bt_relay: %d, state: %Q}", i,
					bool_to_str_state(!pin_read(iot_bt_relay[i].pin.out)));
			return;
		}
	}

	if (gl_drv_led_mqtt == true)
	{
		gl_drv_led_mqtt = false;
		drv_mqtt_pub("{led: %d}", gl_drv_led_pwm);
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
		gl_drv_led_pwm = abs(i);
		if (gl_drv_led_pwm > 100)
			gl_drv_led_pwm = 100;
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
		drv_led_blink_mode(BL_MQTT_SUB_MSG_OK);
		if (MQTT_ACK() == true)
			drv_mqtt_pub("%.*s\n", (int) s->len, s->p);
	}
	else
	{
		drv_led_blink_mode(BL_MQTT_SUB_MSG_ERR);
	}
}

//------------------------------------------------------------------------------
void drv_mqtt_callback(struct mg_connection *c, int ev, void *p, void* user_data)
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
				drv_mqtt_connack = true;
				drv_mqtt_sub();
				drv_led_blink_mode(BL_MQTT_CONNECTED);
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
