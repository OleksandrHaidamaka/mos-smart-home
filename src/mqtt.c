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
 *** MACROSES
 ******************************************************************************/
#define  PUB_TOPIC()    get_cfg()->mqtt.pub
#define  SUB_TOPIC()    get_cfg()->mqtt.sub

/*******************************************************************************
 *** VARIABLES
 ******************************************************************************/
//void (*mqtt_handler)(void) = NULL;
//------------------------------------------------------------------------------
static void mqtt_sub()
{
	struct mg_connection *c = mgos_mqtt_get_global_conn();
	if (c == NULL)
	{
		printf("sub: topic: <error>\n");
		return;
	}

	struct mg_mqtt_topic_expression topic_exp =
	{ SUB_TOPIC(), 0 };
	mg_mqtt_subscribe(c, &topic_exp, 1, 42);
	printf("sub: topic: <%s>\n", SUB_TOPIC());
}

//------------------------------------------------------------------------------
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
	mg_mqtt_publish(c, PUB_TOPIC(), message_id++, MG_MQTT_QOS(0), msg, n);
	printf("pub: topic: <%s> msg: %s\n", PUB_TOPIC(), msg);
}

//------------------------------------------------------------------------------
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
void mqtt_handler(struct mg_connection *c, int ev, void *p)
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
