#ifndef __IOT_H__
#define __IOT_H__

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/
#include <stdbool.h>
#include "drv.h"

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/
#define SHORT_TASK_DELAY         (100 / SYS_TICK)
#define SOS_TASK_DELAY           (350 / SYS_TICK)
#define ALARM_PANIC_MODE_DELAY   (6000 / SYS_TICK)
#define PANIC_TASK_DELAY         (200 / SYS_TICK)

/*******************************************************************************
 *** COMMON ENUM
 ******************************************************************************/
typedef enum
{
	NORMAL_MODE,
	SOS_MODE,
	ALARM_MODE,
	PANIC_MODE,
	NUM_IOT_MODES,
} iot_mode_e;

/*******************************************************************************
 *** COMMON TYPEDEF
 ******************************************************************************/
typedef struct
{
	void (*handler)(void*); // [relay] handler
	int count;
	int state;
	int time;
} task_t;

typedef struct
{
	task_t task;
	iot_mode_e current;
	iot_mode_e requested;
	int timer;
	bool long_press;
	bool pin_state;
} iot_mode_t;

// x - could be switch or button
typedef struct
{
	drv_in_out_t pin;
	drv_mqtt_reason_e mqtt;
	iot_mode_t mode;
	void (*handler)(int); // [x] handler
} iot_x_relay_t;

/*******************************************************************************
 *** PROTOTYPE
 ******************************************************************************/
void iot_x_relay_mode_task_handler(iot_x_relay_t* iot, void (*handler)(void*));
void iot_x_relay_task_sos(void* iot);
void iot_x_relay_task_off_on_alarm(void* iot);
void iot_x_relay_task_panic(void* iot);

#endif  // __IOT_H__
