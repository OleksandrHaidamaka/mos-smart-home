#ifndef __IOT_H__
#define __IOT_H__

/*******************************************************************************
 *** COMMON ENUM
 ******************************************************************************/
typedef enum
{
	NORMAL_MODE, SOS_MODE, ALARM_MODE, PANIC_MODE, SIZE_IOT_MODE,
} iot_mode_e;

/*******************************************************************************
 *** COMMON TYPEDEF
 ******************************************************************************/
typedef struct
{
	void (*handler)(int);
	int count;
	int state;
	int time;
} task_t;

typedef struct
{
	task_t task;
	iot_mode_e cur_name;
	iot_mode_e req_name;
	int timer;
	bool long_press;
	bool pin_state;
} iot_mode_t;

#endif  // __IOT_H__
