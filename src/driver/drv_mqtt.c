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
#define  MQTT_SEND_TIMEOUT  (350/SYS_TICK)  // time [ms]

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    get_cfg()->mqtt.pub
#define  SUB_TOPIC()    get_cfg()->mqtt.sub
#define  MQTT_ACK()     get_cfg()->mqtt.ack

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void drv_mqtt_handler(void);

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
drv_mqtt_t drv_mqtt =
{
	.handler = NULL, .time = 0,
	.mqtt_reason = { "none", "poll", "event" },
	.iot_mode = { "normal", "sos", "alarm", "panic" }
};

//------------------------------------------------------------------------------
static void drv_mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	struct mg_mqtt_topic_expression te =
	{ .topic = SUB_TOPIC(), .qos = 0 };
	uint16_t sub_id = mgos_mqtt_get_packet_id();
	mg_mqtt_subscribe(c, &te, 1, sub_id);
	printf("%s(%s)\n", __func__, SUB_TOPIC());
}

//------------------------------------------------------------------------------
void drv_mqtt_pub(const char *cmd, ...)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	char msg[128];
	struct json_out jmo = JSON_OUT_BUF(msg, sizeof(msg));
	va_list ap;
	int n;
	va_start(ap, cmd);
	n = json_vprintf(&jmo, cmd, ap);
	va_end(ap);
	mg_mqtt_publish(c, PUB_TOPIC(), mgos_mqtt_get_packet_id(), MG_MQTT_QOS(0), msg, n);
	drv_led_blink_mode(BL_MQTT_PUB_MSG);
	printf("%s(%s msg: %s)\n", __func__, PUB_TOPIC(), msg);
}

//------------------------------------------------------------------------------
static char* bool_to_state_str(bool state)
{
	return (state == true ? "ON" : "OFF");
}

//------------------------------------------------------------------------------
static bool state_str_to_bool(char* state)
{
	return (strcmp(state, "ON") ? false : true);
}

//------------------------------------------------------------------------------
static const char* enum_to_mqtt_reason_str(drv_mqtt_reason_e i)
{
	return drv_mqtt.mqtt_reason[(int) i];
}

//------------------------------------------------------------------------------
static const char* enum_to_iot_mode_str(iot_mode_e i)
{
	return drv_mqtt.iot_mode[(int) i];
}

//------------------------------------------------------------------------------
static iot_mode_e iot_mode_str_to_enum(char* mode)
{
	for (int i = 0; i < NUM_IOT_MODES; i++)
		if (strcmp(mode, drv_mqtt.iot_mode[i]) == 0)
		{
			return (iot_mode_e) i;
		}
	return NORMAL_MODE;
}

//------------------------------------------------------------------------------
static void mqtt_led_state(bool state)
{
	if (state == true)
	{
		led_out();
		drv_led.handler = drv_led_handler;
	}
	else
	{
		drv_led.handler = NULL;
		led_in();
	}
}

//------------------------------------------------------------------------------
static void mqtt_relay_state(int i, bool state)
{
	if (i < NUM_IOT_RELAY)
	{
		pin_write(iot_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_sw_mode(int i, iot_mode_e mode_name_new)
{
	if (i < NUM_IOT_SW)
	{
		iot_sw[i].mqtt = POLL;
		iot_bt_relay[i].mode.current = mode_name_new;
	}
}

//------------------------------------------------------------------------------
static void mqtt_sw_relay_state(int i, bool state)
{
	if (i < NUM_IOT_SW_RELAY)
	{
		pin_write(iot_sw_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_bt_relay_state(int i, bool state)
{
	if (i < NUM_IOT_BT_RELAY)
	{
		if (iot_bt_relay[i].mode.current == NORMAL_MODE || iot_bt_relay[i].mode.current == ALARM_MODE)
			pin_write(iot_bt_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_bt_relay_mode(int i, iot_mode_e mode_name_new)
{
	if (i < NUM_IOT_BT_RELAY)
	{
		iot_bt_relay[i].mqtt = POLL;

		switch ((int) mode_name_new)
		{
		case NORMAL_MODE:
			if (iot_bt_relay[i].mode.current == ALARM_MODE)
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_off_on_alarm);
			else
			{
				pin_write(iot_bt_relay[i].pin.out, iot_bt_relay[i].mode.pin_state);
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], NULL);
			}
			break;

		case SOS_MODE:
			if (iot_bt_relay[i].mode.current == NORMAL_MODE)
				iot_bt_relay[i].mode.pin_state = pin_read(iot_bt_relay[i].pin.out);
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_sos);
			break;

		case ALARM_MODE:
			if (iot_bt_relay[i].mode.current == NORMAL_MODE)
				iot_bt_relay[i].mode.pin_state = pin_read(iot_bt_relay[i].pin.out);
			else
				pin_write(iot_bt_relay[i].pin.out, iot_bt_relay[i].mode.pin_state);
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_off_on_alarm);
			break;

		case PANIC_MODE:
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_panic);
			break;
		}

		iot_bt_relay[i].mode.current = iot_bt_relay[i].mode.requested = mode_name_new;
	}
}

//------------------------------------------------------------------------------
static void mqtt_status()
{
	int i;

	for (i = 0; i < NUM_IOT_RELAY; i++)
		iot_relay[i].mqtt = POLL;

	for (i = 0; i < NUM_IOT_SW; i++)
		iot_sw[i].mqtt = POLL;

	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
		iot_sw_relay[i].mqtt = POLL;

	for (i = 0; i < NUM_IOT_BT; i++)
		iot_bt[i].mqtt = POLL;

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
		iot_bt_relay[i].mqtt = POLL;

	drv_led.mqtt = POLL;
}

//------------------------------------------------------------------------------
void drv_mqtt_handler(void)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	if (c == NULL)
	{
		drv_mqtt.time = 0;
		drv_led_blink_mode(BL_WIFI_IP_ACQUIRED);
		return;
	}

	if (drv_mqtt.time++ < MQTT_SEND_TIMEOUT)
		return;

	int i;
	drv_mqtt.time = 0;

	for (i = 0; i < NUM_IOT_RELAY; i++)
	{
		if (iot_relay[i].mqtt != NONE)
		{
			drv_mqtt_pub(
					"{relay: %d, state: %Q, mode_current: %Q, mqtt_reason: %Q}",
					i, bool_to_state_str(!pin_read(iot_relay[i].pin.out)),
					enum_to_iot_mode_str(iot_relay[i].mode.current),
					enum_to_mqtt_reason_str(iot_relay[i].mqtt));
			iot_relay[i].mqtt = NONE;
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW; i++)
	{
		if (iot_sw[i].mqtt != NONE)
		{
			drv_mqtt_pub("{sw: %d, state: %Q, mode_current: %Q, mqtt_reason: %Q}",
					i, bool_to_state_str(!pin_read(iot_sw[i].in)),
					enum_to_iot_mode_str(iot_sw[i].mode.current),
					enum_to_mqtt_reason_str(iot_sw[i].mqtt));
			iot_sw[i].mqtt = NONE;
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
	{
		if (iot_sw_relay[i].mqtt != NONE)
		{
			drv_mqtt_pub(
					"{sw_relay: %d, state: %Q, mode_current: %Q, mode_requested: %Q}",
					i, bool_to_state_str(!pin_read(iot_sw_relay[i].pin.in)),
					enum_to_iot_mode_str(NORMAL_MODE),
					enum_to_iot_mode_str(NORMAL_MODE));
			iot_sw_relay[i].mqtt = NONE;
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT; i++)
	{
		if (iot_bt[i].mqtt != NONE)
		{
			drv_mqtt_pub(
					"{bt: %d, state: %Q, mode_current: %Q, mode_requested: %Q}",
					i, bool_to_state_str(!pin_read(iot_bt[i].in)),
					enum_to_iot_mode_str(NORMAL_MODE),
					enum_to_iot_mode_str(NORMAL_MODE));
			iot_bt[i].mqtt = NONE;
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		if (iot_bt_relay[i].mqtt != NONE)
		{
			drv_mqtt_pub(
					"{bt_relay: %d, state: %Q, mode_current: %Q, mode_requested: %Q, mqtt_reason: %Q}",
					i, bool_to_state_str(!pin_read(iot_bt_relay[i].pin.out)),
					enum_to_iot_mode_str(iot_bt_relay[i].mode.current),
					enum_to_iot_mode_str(iot_bt_relay[i].mode.requested),
					enum_to_mqtt_reason_str(iot_bt_relay[i].mqtt));
			iot_bt_relay[i].mqtt = NONE;
			return;
		}
	}

	if (drv_led.mqtt != NONE)
	{
		drv_mqtt_pub("{led: %Q}", bool_to_state_str(drv_led.handler != NULL));
		drv_led.mqtt = NONE;
	}
}

//------------------------------------------------------------------------------
static void mqtt_parcer_msg(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;
	int i;
	bool err = false;
	char* arg;

	printf("%s(%s msg: %.*s)\n", __func__, SUB_TOPIC(), (int) s->len, s->p);

	/* parsing commands */
	if (json_scanf(s->p, s->len, "{relay: %d, state: %Q}", &i, &arg) == 2)
	{
		mqtt_relay_state(i, state_str_to_bool(arg));
	}
	else if (json_scanf(s->p, s->len, "{sw: %d, mode: %Q}", &i, &arg) == 2)
	{
		mqtt_sw_mode(i, iot_mode_str_to_enum(arg));
	}
	else if (json_scanf(s->p, s->len, "{sw_relay: %d, state: %Q}", &i, &arg) == 2)
	{
		mqtt_sw_relay_state(i, state_str_to_bool(arg));
	}
	else if (json_scanf(s->p, s->len, "{bt_relay: %d, state: %Q}", &i, &arg) == 2)
	{
		mqtt_bt_relay_state(i, state_str_to_bool(arg));
	}
	else if (json_scanf(s->p, s->len, "{bt_relay: %d, mode: %Q}", &i, &arg) == 2)
	{
		mqtt_bt_relay_mode(i, iot_mode_str_to_enum(arg));
	}
	else if (json_scanf(s->p, s->len, "{led: %Q}", &arg) == 1)
	{
		mqtt_led_state(state_str_to_bool(arg));
	}
	else if (strncmp(s->p, "status", sizeof("status") - 1) == 0)
	{
		mqtt_status();
	}
	else
	{
		err = true;
	}

	if (err == false)
	{
		drv_led_blink_mode(BL_MQTT_SUB_MSG_OK);
		if (MQTT_ACK() == true) drv_mqtt_pub("%.*s\n", (int) s->len, s->p);
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
				drv_mqtt.handler = drv_mqtt_handler;
				drv_mqtt_sub();
				mqtt_status();
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

//------------------------------------------------------------------------------
void drv_MQTT_handler(void)
{
	if (drv_mqtt.handler != NULL)
		drv_mqtt.handler();
}
