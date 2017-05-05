#ifndef __CONFIG_H__
#define __CONFIG_H__

/*******************************************************************************
 *** INCLUDES
 ******************************************************************************/

typedef struct
{
	int in;
	int out;
} in_out_t;

/*******************************************************************************
 *** DEFENITIONS
 ******************************************************************************/

//  Pin definitions for NodeMCU (ESP8266)
#define  D0         (16) // IO ONLY!
#define  D1         (5)  // IO
#define  D2         (4)  // IO
#define  D3         (0)  // BOOT_MODE
#define  D4         (2)  // BOOT_MODE - LED_PIN
#define  D5         (14) // IO
#define  D6         (12) // IO
#define  D7         (13) // IO
#define  D8         (15) // BOOT_MODE
#define  D9         (3)  // Rx - I
#define  D10        (1)  // Tx - O

#define  NUM_RELAY_O         (0)  // количество relay объектов
#define  NUM_SW_O            (0)  // количество sw объектов
#define  NUM_SW_RELAY_O      (0)  // количество sw-relay объектов
#define  NUM_BT_O            (0)  // количество bt объектов
#define  NUM_BT_RELAY_O      (2)  // количество bt-relay объектов

/*******************************************************************************
 *** EXTERN VARIABLES
 ******************************************************************************/

#endif  // __CONFIG_H__
