/*
 * ui.h
 *
 *  Created on: Jun 6, 2022
 *      Author: ahmed
 */

#ifndef UI_H_
#define UI_H_

#include"std_types.h"
//define the pass size to be 5 elements which will be stored in array
#define SIZE 5
//hash defines to communicate between the two MCUs
#define EMPTY (0u)
#define SAME (1u)
#define DIFFERENT (2u)
#define OPEN_DOOR (3u)
#define RESET_PASS (4u)


/*********************************************************************************************/
/*                                FUNCTIONS PROTOTYPES                                       */
/*********************************************************************************************/


void APP_init(void);
void APP_sndScreen(void);
uint8 APP_checkPass(uint8 *pass );
uint8 APP_compareSetPass(uint8 *pass,uint8 *pass2);
void APP_getPass(uint8 *pass);
void APP_eppromInit(void);
void APP_uartInit(void);
void epprom_writePass(uint8 *pass);
void eeprom_readPass(uint8 *pass);

#endif /* UI_H_ */
