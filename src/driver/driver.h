#ifndef __DRIVER_H__
#define __DRIVER_H__

/*******************************************************************************
 *** COMMON TYPEDEF
 ******************************************************************************/
typedef struct
{
	void (*off_callback)(int i);
	void (*on_callback)(int i);
	int pin;
	int iot_ind;
	bool state;
} input_t;

typedef struct
{
	int in;
	int out;
} in_out_t;

#endif  // __DRIVER_H__
