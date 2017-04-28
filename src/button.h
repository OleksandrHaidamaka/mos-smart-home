#ifndef __BUTTON_H__
#define __BUTTON_H__

/*******************************************************************************
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	double elapsed_time;
	bool s_new;
	bool s_old;

	void (*on_callback)(int i);
	void (*off_callback)(int i);
} switcher_t;

typedef struct
{
	bool mqtt_update;
} button_relay_t;

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern switcher_t* button;
extern button_relay_t* button_relay;

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void button_init();
void button_driver();

#endif  //__SWITCH_H__
