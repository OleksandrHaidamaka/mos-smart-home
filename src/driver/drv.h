#ifndef __DRIVER_H__
#define __DRIVER_H__

/*******************************************************************************
 *** COMMON TYPEDEF
 ******************************************************************************/
typedef struct
{
	int time_short;
	int time_long;
	int times;
	bool repeat;
} drv_blink_mode_t;

typedef struct
{
	void (*off_callback)(int i);
	void (*on_callback)(int i);
	int pin;
	int iot_ind;
	bool state;
} drv_input_t;

typedef struct
{
	int in;
	int out;
} drv_in_out_t;

#endif  // __DRIVER_H__
