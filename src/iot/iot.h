#ifndef __IOT_H__
#define __IOT_H__

/*******************************************************************************
 *** ENUM
 ******************************************************************************/
typedef enum
{
	NORMAL_MODE, DISCO_MODE, SOS_MODE, ALARM_MODE, SIZE_IOT_MODE
} iot_mode_e;

typedef struct
{
	int state;
	int time;
} stack_t;

typedef struct
{
	stack_t stack;
	iot_mode_e mode;
	int timer;
	bool timer_start;
	bool pin_state;
} iot_mode_t;

#endif  // __IOT_H__
