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
#define  MQTT_SEND_TIMEOUT  (400/SYS_TICK)  // time [ms]
#define  NO_INDEX           (-1)            // индекс объекта отсутствует в сообщении

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    mgos_sys_config_get_mqtt_pub()
#define  SUB_TOPIC()    mgos_sys_config_get_mqtt_sub()
#define  MQTT_ACK()     mgos_sys_config_get_mqtt_ack()
#define  INDEX_OK(i, num)  ((i) >= 0 && (i) < (num))

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
	.mqtt_reason = { "none", "poll", "event", "init" },
	.iot_mode = { "normal", "motion", "sos", "alarm", "panic" }
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
static void drv_mqtt_publish(const char *msg, int len)
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	mg_mqtt_publish(c, PUB_TOPIC(), mgos_mqtt_get_packet_id(), MG_MQTT_QOS(0), msg, len);
	drv_led_blink_mode(BL_MQTT_PUB_MSG);
	printf("%s(%s msg: %.*s)\n", __func__, PUB_TOPIC(), len, msg);
}

//------------------------------------------------------------------------------
void drv_mqtt_pub(const char *cmd, ...)
{
	char msg[128];
	struct json_out jmo = JSON_OUT_BUF(msg, sizeof(msg));
	va_list ap;
	int n;
	va_start(ap, cmd);
	n = json_vprintf(&jmo, cmd, ap);
	va_end(ap);

	/* json_vprintf возвращает полную длину, даже если она не влезла в буфер */
	if (n >= (int) sizeof(msg))
	{
		printf("%s: message is too long (%d bytes)\n", __func__, n);
		return;
	}
	drv_mqtt_publish(msg, n);
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
	return NUM_IOT_MODES; // неизвестный режим
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
	drv_led.mqtt = POLL;
}

//------------------------------------------------------------------------------
static bool mqtt_relay_state(int i, bool state)
{
	if (INDEX_OK(i, NUM_IOT_RELAY))
	{
		pin_write(iot_relay[i].pin.out, !state);
		iot_relay[i].mqtt = POLL;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static bool mqtt_relay_mode(int i, iot_mode_e mode_name_new)
{
	if (INDEX_OK(i, NUM_IOT_RELAY) && mode_name_new < NUM_IOT_MODES)
	{
		iot_relay[i].mqtt = POLL;

		switch ((int) mode_name_new)
		{
		case NORMAL_MODE:
			switch ((int) iot_relay[i].mode.current)
			{
			case SOS_MODE:
				pin_write(iot_relay[i].pin.out, iot_relay[i].mode.pin_state);
				iot_x_relay_mode_task_handler(&iot_relay[i], NULL);
				break;

			case ALARM_MODE:
				iot_relay[i].mode.pin_state = false;
				iot_x_relay_mode_task_handler(&iot_relay[i], iot_x_relay_task_off_on_blink);
				break;

			case PANIC_MODE:
				pin_write(iot_relay[i].pin.out, false);
				iot_x_relay_mode_task_handler(&iot_relay[i], NULL);
				break;

			case MOTION_MODE:
				iot_relay[i].mode.pin_state = true;
				iot_x_relay_mode_task_handler(&iot_relay[i], iot_x_relay_task_off_on_blink);
				break;
			}
			break;

		case SOS_MODE:
			// сохраняем состояние только если реле не мигает (иначе оно случайное)
			if (iot_relay[i].mode.task.handler == NULL)
				iot_relay[i].mode.pin_state = pin_read(iot_relay[i].pin.out);
			iot_x_relay_mode_task_handler(&iot_relay[i], iot_x_relay_task_sos);
			break;

		case ALARM_MODE: case MOTION_MODE:
			iot_relay[i].mode.pin_state = true;
			iot_x_relay_mode_task_handler(&iot_relay[i], iot_x_relay_task_off_on_blink);
			break;

		case PANIC_MODE:
			iot_x_relay_mode_task_handler(&iot_relay[i], iot_x_relay_task_panic);
			break;
		}

		iot_relay[i].mode.current = mode_name_new;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static bool mqtt_sw_mode(int i, iot_mode_e mode_name_new)
{
	if (INDEX_OK(i, NUM_IOT_SW) && mode_name_new < NUM_IOT_MODES)
	{
		iot_sw[i].mqtt = POLL;
		iot_sw[i].mode.current = mode_name_new;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static bool mqtt_sw_relay_state(int i, bool state)
{
	if (INDEX_OK(i, NUM_IOT_SW_RELAY))
	{
		pin_write(iot_sw_relay[i].pin.out, !state);
		iot_sw_relay[i].mqtt = true;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static bool mqtt_bt_relay_state(int i, bool state)
{
	if (INDEX_OK(i, NUM_IOT_BT_RELAY))
	{
		switch((int) iot_bt_relay[i].mode.current)
		{
		case NORMAL_MODE: case MOTION_MODE: case ALARM_MODE:
			pin_write(iot_bt_relay[i].pin.out, !state);
			break;
		}
		iot_bt_relay[i].mqtt = POLL;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static bool mqtt_bt_relay_mode(int i, iot_mode_e mode_name_new)
{
	if (INDEX_OK(i, NUM_IOT_BT_RELAY) && mode_name_new < NUM_IOT_MODES)
	{
		iot_bt_relay[i].mqtt = POLL;

		switch ((int) mode_name_new)
		{
		case NORMAL_MODE:
			switch ((int) iot_bt_relay[i].mode.current)
			{
			case SOS_MODE:
				pin_write(iot_bt_relay[i].pin.out, iot_bt_relay[i].mode.pin_state);
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], NULL);
				break;

			case ALARM_MODE:
				iot_bt_relay[i].mode.pin_state = false;
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_off_on_blink);
				break;

			case PANIC_MODE:
				pin_write(iot_bt_relay[i].pin.out, false);
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], NULL);
				break;

			case MOTION_MODE:
				iot_bt_relay[i].mode.pin_state = true;
				iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_off_on_blink);
				break;
			}
			break;

		case SOS_MODE:
			// сохраняем состояние только если реле не мигает (иначе оно случайное)
			if (iot_bt_relay[i].mode.task.handler == NULL)
				iot_bt_relay[i].mode.pin_state = pin_read(iot_bt_relay[i].pin.out);
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_sos);
			break;

		case ALARM_MODE: case MOTION_MODE:
			iot_bt_relay[i].mode.pin_state = true;
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_off_on_blink);
			break;

		case PANIC_MODE:
			iot_x_relay_mode_task_handler(&iot_bt_relay[i], iot_x_relay_task_panic);
			break;
		}

		iot_bt_relay[i].mode.current = iot_bt_relay[i].mode.requested = mode_name_new;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------
static void mqtt_iot_x_poll()
{
	int i;

	drv_mqtt.mqtt_reason_e = INIT;
	for (i = 0; i < NUM_IOT_RELAY; i++)
		iot_relay[i].mqtt = POLL;
	for (i = 0; i < NUM_IOT_SW; i++)
		iot_sw[i].mqtt = POLL;
	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
		iot_sw_relay[i].mqtt = true;
	for (i = 0; i < NUM_IOT_BT; i++)
		iot_bt[i].mqtt = true;
	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
		iot_bt_relay[i].mqtt = POLL;
	drv_led.mqtt = POLL;
}

//------------------------------------------------------------------------------
static void mqtt_iot_x_init()
{
	iot_relay_init();
	iot_switch_init();
	iot_switch_relay_init();
	iot_button_init();
	iot_button_relay_init();
	mqtt_iot_x_poll();
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

	if(drv_mqtt.mqtt_reason_e == INIT)
	{
		drv_mqtt_pub("init");
		drv_mqtt.mqtt_reason_e = NONE;
	}

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
					i, bool_to_state_str(!pin_read(iot_sw_relay[i].pin.out)),
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
static bool mqtt_is_init_cmd(const struct mg_str *s)
{
	size_t len = s->len;

	/* допускаем завершающие пробелы и перевод строки */
	while (len > 0 && (s->p[len - 1] == ' ' || s->p[len - 1] == '\r'
			|| s->p[len - 1] == '\n'))
		len--;

	return (len == sizeof("init") - 1 && strncmp(s->p, "init", len) == 0);
}

//------------------------------------------------------------------------------
static void mqtt_parcer_msg(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;
	int relay = NO_INDEX, sw = NO_INDEX, sw_relay = NO_INDEX, bt_relay = NO_INDEX;
	char *state = NULL, *mode = NULL, *led = NULL;
	bool ok;

	printf("%s(%s msg: %.*s)\n", __func__, SUB_TOPIC(), (int) s->len, s->p);

	/* все ключи разбираются за один проход;
	 строки %Q выделяются через malloc и освобождаются ниже */
	json_scanf(s->p, s->len,
			"{relay: %d, sw: %d, sw_relay: %d, bt_relay: %d, state: %Q, mode: %Q, led: %Q}",
			&relay, &sw, &sw_relay, &bt_relay, &state, &mode, &led);

	/* parsing commands */
	if (relay != NO_INDEX && state != NULL)
		ok = mqtt_relay_state(relay, state_str_to_bool(state));
	else if (relay != NO_INDEX && mode != NULL)
		ok = mqtt_relay_mode(relay, iot_mode_str_to_enum(mode));
	else if (sw != NO_INDEX && mode != NULL)
		ok = mqtt_sw_mode(sw, iot_mode_str_to_enum(mode));
	else if (sw_relay != NO_INDEX && state != NULL)
		ok = mqtt_sw_relay_state(sw_relay, state_str_to_bool(state));
	else if (bt_relay != NO_INDEX && state != NULL)
		ok = mqtt_bt_relay_state(bt_relay, state_str_to_bool(state));
	else if (bt_relay != NO_INDEX && mode != NULL)
		ok = mqtt_bt_relay_mode(bt_relay, iot_mode_str_to_enum(mode));
	else if (led != NULL)
	{
		mqtt_led_state(state_str_to_bool(led));
		ok = true;
	}
	else if (mqtt_is_init_cmd(s))
	{
		mqtt_iot_x_init();
		ok = true;
	}
	else
	{
		ok = false;
	}

	free(state);
	free(mode);
	free(led);

	if (ok == true)
	{
		drv_led_blink_mode(BL_MQTT_SUB_MSG_OK);
		if (MQTT_ACK() == true) drv_mqtt_publish(s->p, (int) s->len);
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
				mqtt_iot_x_poll();
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
