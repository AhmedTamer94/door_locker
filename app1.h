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
void APP_firstTimePass(uint8 *pass);
uint8 APP_checkPass(void);
void APP_setPass(uint8 *pass);
void APP_reenterPass(uint8 *pass);
uint8 APP_askPass(uint8 *pass);
void APP_enterPass(uint8 *pass);
void APP_sendPass(uint8 *pass);
void APP_sndScreen(void);
void APP_resetPass(uint8 *pass);
void APP_doorInit(void);
void APP_lock1MinInit(void);
void APP_lock1Min(void);
void APP_openDoor(void);
void APP_uartInit(void);

#endif /* UI_H_ */
