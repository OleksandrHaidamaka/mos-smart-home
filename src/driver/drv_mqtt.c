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
#define  MQTT_SEND_TIMEOUT  (300/SYS_TICK)  // time [ms]

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    get_cfg()->mqtt.pub
#define  SUB_TOPIC()    get_cfg()->mqtt.sub
#define  MQTT_ACK()     get_cfg()->mqtt.ack

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
const char* iot_mode_str[] =
{ "normal", "disco", "sos", "alarm", "panic" };

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
static const char* ind_to_mode_str(iot_mode_e i)
{
	return iot_mode_str[(int) i];
}

//------------------------------------------------------------------------------
static iot_mode_e mode_str_to_ind(char* mode)
{
	for (int i = 0; i < SIZE_IOT_MODE; i++)
		if (strcmp(mode, iot_mode_str[i]) == 0)
		{
			return (iot_mode_e) i;
		}
	return NORMAL_MODE;
}

//------------------------------------------------------------------------------
static void mqtt_relay_state(int i, bool state)
{
	if (i < NUM_IOT_RELAY)
	{
		pin_write(iot_relay[i].out, !state);
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
		if (iot_bt_relay[i].mode.name == NORMAL_MODE
				|| iot_bt_relay[i].mode.name == ALARM_MODE)
			pin_write(iot_bt_relay[i].pin.out, !state);
	}
}

//------------------------------------------------------------------------------
static void mqtt_bt_relay_mode(int i, char* mode)
{
	if (i < NUM_IOT_BT_RELAY)
	{
		iot_mode_e mode_name_new = mode_str_to_ind(mode);
		iot_bt_relay[i].mqtt = false;

		switch ((int) mode_name_new)
		{
		case NORMAL_MODE:
			if (iot_bt_relay[i].mode.name == ALARM_MODE)
				iot_button_relay_mode_task_handler(i,
						iot_button_relay_task_go_to_normal);
			else
			{
				iot_button_relay_mode_task_handler(i, NULL);
				pin_write(iot_bt_relay[i].pin.out,
						iot_bt_relay[i].mode.pin_state);
				iot_bt_relay[i].mqtt = true;
			}
			break;

		case SOS_MODE:
			if (iot_bt_relay[i].mode.name == NORMAL_MODE)
				iot_bt_relay[i].mode.pin_state = pin_read(
						iot_bt_relay[i].pin.out);
			iot_button_relay_mode_task_handler(i,
					iot_button_relay_task_sos_panic);
			break;

		case ALARM_MODE:
			iot_button_relay_mode_task_handler(i, NULL);
			if (iot_bt_relay[i].mode.name == NORMAL_MODE)
				iot_bt_relay[i].mode.pin_state = pin_read(
						iot_bt_relay[i].pin.out);
			else
				pin_write(iot_bt_relay[i].pin.out,
						iot_bt_relay[i].mode.pin_state);
			break;

		case PANIC_MODE:
			iot_button_relay_mode_task_handler(i,
					iot_button_relay_task_sos_panic);
			break;
		}

		iot_bt_relay[i].mode.name = mode_name_new;
	}
}

//------------------------------------------------------------------------------
static void mqtt_status()
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
			drv_mqtt_pub(
					"{relay: %d, state: %Q, mode_current: %Q, mode_requsted: %Q}",
					i, bool_to_str_state(!pin_read(iot_relay[i].out)),
					ind_to_mode_str(NORMAL_MODE), ind_to_mode_str(NORMAL_MODE));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW; i++)
	{
		if (iot_sw[i].mqtt == true)
		{
			iot_sw[i].mqtt = false;
			drv_mqtt_pub(
					"{sw: %d, state: %Q, mode_current: %Q, mode_requsted: %Q}",
					i, bool_to_str_state(!pin_read(iot_sw[i].in)),
					ind_to_mode_str(NORMAL_MODE), ind_to_mode_str(NORMAL_MODE));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_SW_RELAY; i++)
	{
		if (iot_sw_relay[i].mqtt == true)
		{
			iot_sw_relay[i].mqtt = false;
			drv_mqtt_pub(
					"{sw_relay: %d, state: %Q, mode_current: %Q, mode_requsted: %Q}",
					i, bool_to_str_state(!pin_read(iot_sw_relay[i].pin.in)),
					ind_to_mode_str(NORMAL_MODE), ind_to_mode_str(NORMAL_MODE));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT; i++)
	{
		if (iot_bt[i].mqtt == true)
		{
			iot_bt[i].mqtt = false;
			drv_mqtt_pub("{bt: %d, state: %Q, %Q}", i,
					bool_to_str_state(!pin_read(iot_bt[i].in)),
					ind_to_mode_str(NORMAL_MODE), ind_to_mode_str(NORMAL_MODE));
			return;
		}
	}

	for (i = 0; i < NUM_IOT_BT_RELAY; i++)
	{
		if (iot_bt_relay[i].mqtt == true)
		{
			iot_bt_relay[i].mqtt = false;

			if (iot_bt_relay[i].mode.long_press == true)
				drv_mqtt_pub(
						"{bt_relay: %d, state: %Q, mode_current: %Q, mode_requsted: %Q}",
						i,
						bool_to_str_state(!pin_read(iot_bt_relay[i].pin.out)),
						ind_to_mode_str(iot_bt_relay[i].mode.name),
						ind_to_mode_str(NORMAL_MODE));
			else
				drv_mqtt_pub(
						"{bt_relay: %d, state: %Q, mode_current: %Q, mode_requsted: %Q}",
						i,
						bool_to_str_state(!pin_read(iot_bt_relay[i].pin.out)),
						ind_to_mode_str(iot_bt_relay[i].mode.name),
						ind_to_mode_str(iot_bt_relay[i].mode.name));
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
	char* arg;

	printf("%s(%s msg: %.*s)\n", __func__, SUB_TOPIC(), (int) s->len, s->p);

	/* parsing commands */
	if (json_scanf(s->p, s->len, "{relay: %d, state: %Q}", &i, &arg) == 2)
	{
		mqtt_relay_state(i, str_to_bool_state(arg));
	}
	else if (json_scanf(s->p, s->len, "{sw_relay: %d, state: %Q}", &i, &arg)
			== 2)
	{
		mqtt_sw_relay_state(i, str_to_bool_state(arg));
	}
	else if (json_scanf(s->p, s->len, "{bt_relay: %d, state: %Q}", &i, &arg)
			== 2)
	{
		mqtt_bt_relay_state(i, str_to_bool_state(arg));
	}
	else if (json_scanf(s->p, s->len, "{bt_relay: %d, mode: %Q}", &i, &arg)
			== 2)
	{
		mqtt_bt_relay_mode(i, arg);
	}
	else if (json_scanf(s->p, s->len, "{led: %d}", &i) == 1)
	{
		gl_drv_led_pwm = abs(i);
		if (gl_drv_led_pwm > 100)
			gl_drv_led_pwm = 100;
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
		if (MQTT_ACK() == true)
			drv_mqtt_pub("%.*s\n", (int) s->len, s->p);
	}
	else
	{
		drv_led_blink_mode(BL_MQTT_SUB_MSG_ERR);
	}
}

//------------------------------------------------------------------------------
void drv_mqtt_callback(struct mg_connection *c, int ev, void *p,
		void* user_data)
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
