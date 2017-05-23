#ifndef __IOT_SW_RELAY_H__
#define __IOT_SW_RELAY_H__

/*******************************************************************************
 *** TYPEDEF
 ******************************************************************************/
typedef struct
{
	drv_in_out_t pin;
	bool mqtt;
} iot_sw_relay_t;

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/
extern iot_sw_relay_t iot_sw_relay[NUM_IOT_SW_RELAY];

/*******************************************************************************
 *** PROTOTYPES
 ******************************************************************************/
void iot_switch_relay_init(void);
void iot_switch_relay_off_callback(int i);
void iot_switch_relay_on_callback(int i);

#endif  //__IOT_SW_RELAY_H__
