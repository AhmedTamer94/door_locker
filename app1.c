/*
 * ui.c
 *
 *  Created on: Jun 6, 2022
 *      Author: ahmed
 */

#include"lcd.h"
#include"keypad.h"
#include"uart.h"
#include"app.h"
#include"timer.h"
#include"util/delay.h"
//global variable for the timer counter
static volatile uint8 tck=1;
// timer structure configuration
extern TIMER_configType timer;

/*this function initiates the application modules LCD and UART and the global interrupt
 * this ask the user to enter a pass after that sends the pass via uart and waits the 2nd MCU
 * signal. if the signal = (empty(0) it will set the user first password, if same(1) it
 * will display the 2nd screen but if different(2) will ask the user to enter the pass again
 * two more times in each time will send the pass to the 2nd MCU to check on them if the pass
 * doesn't match it will trigger display a msg (!!!THIEF!!!) on LCD then returns again to 2nd
 * screen
 */
void APP_init(void){
	//variables declarations
	uint8 condition,counter=0,pass[SIZE];
	//enable global interrupt
	SREG=(1<<7);
	//initiate LCD
	LCD_init();
	//initiate UART
	APP_uartInit();
	//function that display enter a pass and gets that pass
	APP_enterPass(pass);
	//send the pass by UART
	APP_sendPass(pass);
	/*waits the 2nd MCU to send if the epprom is empty or the epprom has a pass and it matches
	or it doesn't match the pass*/
	condition=APP_checkPass();
	if(condition==EMPTY){
		//if epprom is empty then set the first pass for the user
		APP_firstTimePass(pass);
		return;
	}

	else if (condition== SAME){
		//do nothing and exit the function
		return;
	}
	else{
		/*if it doesn't match try two more times and it doesnt match display a msg(!!!THIEF!!!)
		 * on the LCD
		 */
		counter++;
		LCD_clearScreen();
		LCD_displayString("WRONG PASS");
		_delay_ms(1000);
		//this APP_askPass get a pass from the user and waits the condition from 2nd MCU
		while((APP_askPass(pass)==DIFFERENT)&&(counter<2)){
			counter++;
		}
		if(counter==2){
			//if the three tries were wrong display (!!!THIEF!!!) for 1 min
			APP_lock1MinInit();
		}
	}
}
/*when the MCU get to this function that is means the epprom has no pass on it
 * so display there's no pass and set user pass and take the pass two times if it matches
 * go to the 2nd screen if not matches repeat takeing the pass again
 */
void APP_firstTimePass(uint8 *pass){
	//display msg no LCD that no pass on epprom
	LCD_clearScreen();
	LCD_displayString("NO PASS PLEASE");
	LCD_moveCursor(1,0);
	LCD_displayString("SET A PASS");
	_delay_ms(1000);
	//enter the pass first time
	APP_setPass(pass);
	//send the first pass by uart
	APP_sendPass(pass);
	//enter again pass
	APP_reenterPass(pass);
	//send it by uart
	APP_sendPass(pass);
	/*if the two pass doesn't match (retrun of checkPass function == DIFFERENT(2) repeat taking
	 * the pass and display the two pass doesn't match
	 */
	while(APP_checkPass()==DIFFERENT){
		LCD_clearScreen();
		LCD_displayString("PASS NOT MATCH");
		_delay_ms(1000);
		APP_setPass(pass);
		APP_sendPass(pass);
		APP_reenterPass(pass);
		APP_sendPass(pass);
	}
}

void APP_sndScreen(void){
	//variables declaration
	uint8 key,pass[SIZE],counter=0;
	LCD_clearScreen();
	//display the two option that this app can do
	LCD_displayString("+: OPEN DOOR");
	LCD_moveCursor(1,0);
	LCD_displayString("-: Change PASS");

	//get the kepadkey value and store in key variable
	key=KEYPAD_getPressedKey();

	//while key doesn't equal to ASCII code of '+' or '-' repeat taking the keypad value
	while((key!='+')&&(key!='-')){
		key=KEYPAD_getPressedKey();
	}
	/* if key equal to '+' then send the value by uart and ask for the pass 3 times as a try
	 * if the user didn't enter the correct pass display (!!!THIEF!!!) for 1 min
	 */
	if(key=='+'){
		UART_sendByte(OPEN_DOOR);
		while((APP_askPass(pass)==DIFFERENT)&&(counter<2)){
			counter++;
		}
		if(counter==2){
			//display (!!!THIEF!!!) for 1 min
			APP_lock1MinInit();
		}
		else{
			//display door is opening for 15s and closing for 15s
			APP_doorInit();
		}
	}
	/* if key equal to '-' then send the value by uart and ask for the pass 3 times as a try
		 * if the user didn't enter the correct pass display (!!!THIEF!!!) for 1 min
		 */
	else if (key=='-'){
		UART_sendByte(RESET_PASS);
		while((APP_askPass(pass)==DIFFERENT)&&(counter<2)){
			counter++;
		}
		if(counter==2){
			//display (!!!THIEF!!!) for 1 min
			APP_lock1MinInit();
		}
		else{
			//reset pass
			APP_resetPass(pass);
		}
	}
}

//get the pass condition from the 2nd MCU and return this value
uint8 APP_checkPass(void){
	return UART_recieveByte();
}
/*get the pass and send it to the 2nd MCU and waits the pass condition from the 2nd MCU
 * if different display on LCD pass wrong an return this condition if same do nothing and
 * also return this condition
 */
uint8 APP_askPass(uint8 *pass){
	APP_enterPass(pass);
	APP_sendPass(pass);
	if(APP_checkPass()==DIFFERENT){
		LCD_clearScreen();
		LCD_displayString("PASS WRONG");
		_delay_ms(1000);
		return DIFFERENT;
	}
	else
		return SAME;
}
/*display RE-SET pass and waits the user to enter the full pass (5 values)
 *
 */
void APP_reenterPass(uint8 *pass){
	uint8 i;
	LCD_clearScreen();
	LCD_displayString("RE-SET Pass:");
	LCD_moveCursor(1,0);
	for(i=0;i<SIZE;i++){
		/*if the keypad value>9 do nothing excpet the ASCII of enter (13) (AC/on) on keypad
		 * if pressed then the user will delete the last number and replace it with another
		 * number
		 */
		while(KEYPAD_getPressedKey()>9){
			if(KEYPAD_getPressedKey()==13){
				if(i){
					LCD_moveCursor(1,i-1);
					LCD_displayCharacter(' ');
					LCD_moveCursor(1,i-1);
					i--;
					break;
				}

			}
		}
		pass[i]=KEYPAD_getPressedKey();
		/*if user pressed enter AC/on then he wants to delet the last number he inputs and
		rewrite on this value*/
		if(pass[i]==13)
			i--;
		//display '*' instead of the number
		else
			LCD_displayCharacter('*');
		//simulate the user press to not get the same value more than one time
		_delay_ms(500);
	}
	//do nothing and waits the user to press '='
	while(KEYPAD_getPressedKey()!='=');
}

//the same function of reenterpass put the LCD will display Set your pass
void APP_setPass(uint8 *pass){
	uint8 i;
	LCD_clearScreen();
	LCD_displayString("SET Your Pass:");
	LCD_moveCursor(1,0);
	for(i=0;i<SIZE;i++){
		while(KEYPAD_getPressedKey()>9){
			if(KEYPAD_getPressedKey()==13){
				if(i){
					LCD_moveCursor(1,i-1);
					LCD_displayCharacter(' ');
					LCD_moveCursor(1,i-1);
					i--;
					break;
				}
			}
		}
		pass[i]=KEYPAD_getPressedKey();
		if(pass[i]==13)
			i--;
		else
			LCD_displayCharacter('*');
		_delay_ms(500);
	}
	while(KEYPAD_getPressedKey()!='=');
}

//the same function of reenterpass put the LCD will display Enter your pass
void APP_enterPass(uint8 *pass){
	uint8 i;
	LCD_clearScreen();
	LCD_displayString("Enter Your Pass:");
	LCD_moveCursor(1,0);
	for(i=0;i<SIZE;i++){
		while(KEYPAD_getPressedKey()>9){
			if(KEYPAD_getPressedKey()==13){
				if(i){
					LCD_moveCursor(1,i-1);
					LCD_displayCharacter(' ');
					LCD_moveCursor(1,i-1);
					i--;
					break;
				}

			}
		}
		pass[i]=KEYPAD_getPressedKey();
		if(pass[i]==13)
			i--;
		else
			LCD_displayCharacter('*');
		_delay_ms(500);
	}
	while(KEYPAD_getPressedKey()!='=');
}
//send the pass using uart
void APP_sendPass(uint8 *pass){
	uint8 i;
	for(i=0;i<SIZE;i++){
		UART_sendByte(pass[i]);
	}
}
//this function reset the pass
void APP_resetPass(uint8 *pass){
	//takes the pass
	APP_setPass(pass);
	//send the pass
	APP_sendPass(pass);
	//retake the pass
	APP_reenterPass(pass);
	//resend the pass
	APP_sendPass(pass);
	//if the 2nd MCU send DIFFERENT display pass not match and repeat the function till they
	//matches
	while(APP_checkPass()==DIFFERENT){
		LCD_clearScreen();
		LCD_displayString("PASS NOT MATCH");
		_delay_ms(1000);
		APP_setPass(pass);
		APP_sendPass(pass);
		APP_reenterPass(pass);
		APP_sendPass(pass);
	}
	//tell the user the operation has been done and success
	LCD_clearScreen();
	LCD_displayString("PASS HAS SET");
	_delay_ms(1000);
}
//initiate the timer and display door is opening for 15s
void APP_doorInit(void){
	TIMER_init(&timer);
	//set the call back function
	TIMER_SET_callBack(APP_openDoor);
	LCD_clearScreen();
	LCD_displayString("DOOR IS OPENING");
	//do nothing till the whole 30s
	while(tck);
	//reset the value
	tck=1;
}
//initiate the timer and display (!!!THIEF!!!) for 1min
void APP_lock1MinInit(void){
	TIMER_init(&timer);
	//set the call back function
	TIMER_SET_callBack(APP_lock1Min);
	LCD_clearScreen();
	LCD_displayString("!!!THIEF!!!");
	//do nothing till the whole 1min
	while(tck);
	//reset the value
	tck=1;
}

/*if the tck reaches 20 then the time has ended disable timer interrupt
and set the tck=0 to break while(tck);*/
void APP_lock1Min(void){
	if(tck<19)
		tck++;
	else{
		TIMER_DISABLE(&timer);
		tck=0;
	}
}
/*if the tck==5 then the time passed=15s then display door is closing
 * and if the tck=11 then the time passed =30s (the 2nd 15s)
 * then disable timer interrupt and set the tck=0 to break while(tck);*/
void APP_openDoor(void){
	if(tck==5){
		tck++;
		LCD_clearScreen();
		LCD_displayString("DOOR IS ClOSING");
	}
	else if(tck==11){
		tck=0;
		TIMER_DISABLE(&timer);
	}
	else{
		tck++;
	}
}
//initiates the uart
void APP_uartInit(void){
	UART_config uart={FRAME_8_BITS,PARITY_EVEN,STOP_BITS_1,RISING,19200};
	UART_init(&uart);
}


