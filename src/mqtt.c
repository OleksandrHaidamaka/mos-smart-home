/*
 @autor:       Alexandr Haidamaka
 @file:        mqtt.c
 @description: MQTT functionality
 */

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include "main.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define  MQTT_SEND_TIMEOUT   (1000/SYS_TICK)  // time [ms]

/*******************************************************************************
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    get_cfg()->mqtt.pub
#define  SUB_TOPIC()    get_cfg()->mqtt.sub

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
void (*mqtt_callback)(void) = NULL;

//------------------------------------------------------------------------------
static void mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();

	struct mg_mqtt_topic_expression topic_exp =
	{ SUB_TOPIC(), 0 };
	mg_mqtt_subscribe(c, &topic_exp, 1, 42);
	printf("sub: topic: <%s>\n", SUB_TOPIC());
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
	printf("pub: topic: <%s> msg: %s\n", PUB_TOPIC(), msg);
	blink_mode(BL_MQTT_PUB);
}

//------------------------------------------------------------------------------
static void mqtt_light_id(int id, bool state)
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

//------------------------------------------------------------------------------
static void mqtt_cmd_parcer(struct mg_mqtt_message* msg)
{
	struct mg_str *s = &msg->payload;

	int id, state;

	printf("sub: topic: <%s> ", SUB_TOPIC());
	if (json_scanf(s->p, s->len, "{light_id: %d, state: %d}", &id, &state) == 2)
	{
		printf("msg: %.*s\n", (int) s->len, s->p);
		mqtt_light_id(id, (bool) state);
	}
	else
	{
		printf("msg: <unsupported>\n");
	}
}

//------------------------------------------------------------------------------
void mqtt_manager()
{
	static int time = 0;

	if (time++ < MQTT_SEND_TIMEOUT)
		return;
	time = 0;

	struct mg_connection *c = mgos_mqtt_get_global_conn();

	if (c == NULL)
	{
		mqtt_callback = NULL;
		blink_mode(BL_WIFI_IP_ACQUIRED);
		return;
	}

	for (int i = 0; i < NUM_NODES; i++)
	{
		if (switch_state[i].update == true)
		{
			switch_state[i].update = false;
			mqtt_pub("{light_id: %d, state: %d}", LIGHT_ID(i),
					switch_state[i].s_old);
			return;
		}
	}
}

//------------------------------------------------------------------------------
void mqtt_handler(struct mg_connection *c, int ev, void *p)
{
	(void) c;
	struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

	switch (ev)
	{
	case MG_EV_MQTT_CONNACK:
		if (msg->connack_ret_code == 0)
		{
			if (PUB_TOPIC() != NULL && SUB_TOPIC() != NULL)
			{
				mqtt_sub();
				mqtt_callback = mqtt_manager;
				blink_mode(BL_MQTT_CONNECTED);
			}
			else
				printf("Run 'mos config-set mqtt.sub=... mqtt.pub=...'\n");
		}
		break;
	case MG_EV_MQTT_PUBLISH:
		mqtt_cmd_parcer(msg);
		blink_mode(BL_MQTT_SUB);
		break;
	}
}
