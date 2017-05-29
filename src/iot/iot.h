#ifndef __IOT_H__
#define __IOT_H__

/*******************************************************************************
 *** ENUM
 ******************************************************************************/
typedef enum
{
	NORMAL_MODE, DISCO_MODE, SOS_MODE, ALARM_MODE, PANIC_MODE, SIZE_IOT_MODE,
} iot_mode_e;

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
	iot_mode_e name;
	int timer;
	bool long_press;
	bool pin_state;
} iot_mode_t;

#endif  // __IOT_H__
