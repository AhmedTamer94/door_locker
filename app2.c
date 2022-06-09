/*
 * ui.c
 *
 *  Created on: Jun 6, 2022
 *      Author: ahmed
 */
#include"external_eeprom.h"
#include"uart.h"
#include"app.h"
#include"util/delay.h"
#include"twi.h"
#include"motor.h"
#include"buzzer.h"
#include"avr/interrupt.h"

/*this function initiates the application modules TWI, UART, Motor and BUZZER
 * and the global interrupt then waits the 1st MCU to send a pass via uart then read the epprom
 * value then sends the condition to 1st MCU= (if empty it will send empty(0) to 1st MCU,
 * if same send(1), if different send(2)) and if different it will repeat the same operation
 * two more times in each time get the pass from 1st MCU and read epprom value store in it if
 * doesn't match in the third time it will trigger the BUZZER for 1min then return to 2nd
 * screen function
 */
void APP_init(void){
	//variables declarations
	uint8 condition,pass[SIZE], pass2[SIZE],counter=0;
	//enable global interrupt
	SREG=(1<<7);
	//init epprom TWI
	APP_eppromInit();
	//init uart
	APP_uartInit();
	//init motor
	DcMotor_init();
	//init buzzzer
	BUZZER_init();
	//waits the 1st MCU to send the pass
	APP_getPass(pass);
	//read the value stored in epprom and send the condtion to 1st MCU and return this value
	//to be stored in variable condition
	condition=APP_checkPass(pass);
	if(condition==EMPTY){
		//if empty get the pass two times and check on the two pass
		APP_getPass(pass);
		APP_getPass(pass2);
		//if different send different to 1st and repeats the operation
		while(APP_compareSetPass(pass,pass2)==DIFFERENT){
			APP_getPass(pass);
			APP_getPass(pass2);
		}
		//when they are identical store in in eeprom
		epprom_writePass(pass);
		return;
	}
	//if the epprom has a pass and the pass sent from 1st MCU are the same exit the function
	else if(condition==SAME){
		return;
	}
	else{
		/*if the epprom has a pass and the pass sent from 1st MCU are the different
		 * repeat two more times get the pass from 1st MCU and read the value from epprom
		 * if not the same after the 2nd time trial turn on buzzer for 1min if it the same
		 * in less than two times trial exit the function
		 */
		counter++;
		APP_getPass(pass);
		while((APP_checkPass(pass)==DIFFERENT)&&counter<2){
			counter++;
			APP_getPass(pass);
		}
		if(counter==2){
			BUZZER_on();
		}
	}
}
/*2nd function recieve the functionality from uart (oppen door or reset pass) the recieve
 * the pass. if the pass is the same in the epprom do the functionality (open the door in 15s
 * then close it in 15s, or reset the pass).
 * if the pass doesn't match after three trials turn on the buzzer for 1 min
 */
void APP_sndScreen(void){
	//variables declarations
	uint8 condition,counter=0,pass[SIZE],pass2[SIZE];
	//recieve the functionality to be done and store in condition variable
	condition=UART_recieveByte();
	//recieve the pass
	APP_getPass(pass);
	/*chech the pass and send the condition (same or diff) by uart if the pass doesn't match
	 * for three times turn on the buzzer, else do the function required
	 */
	while((APP_checkPass(pass)==DIFFERENT)&&counter<2){
		counter++;
		APP_getPass(pass);
	}
	//if condition==open door
	if(condition== OPEN_DOOR){
		//if the pass doesn't match for three times
		if(counter==2){
			//turn on buzzer
			BUZZER_on();
		}
		else{
			//turn on motor
			DcMotor_on();
		}
	}
	else {//if condition==reset
		//if the pass doesn't match for three times
		if(counter==2){
			//turn on buzzer
			BUZZER_on();

		}
		else{
			//reset the pass if counter<2
			APP_getPass(pass);
			APP_getPass(pass2);
			//check on the two pass if they are doesn't match send diff to 1st MCU and repeat
			//till they match
			while(APP_compareSetPass(pass,pass2)==DIFFERENT){
				APP_getPass(pass);
				APP_getPass(pass2);
			}
			//write the new pass on epprom
			epprom_writePass(pass);
		}
	}

}
/*this function check on the pass as it read the value stored in epprom then compare it to the
 * pass sent by uart if the epprom first use in the default value will be (255)>9 then send
 * empty to 1st MCU and return this value if the stored value is equal or less than 9
 * then the epprom has a pass on it if its the same return same and pass is identical if
 * not the pass is different and return different
 */
uint8 APP_checkPass(uint8 *pass){
	//variables declarations
	uint8 i,count=0,data[SIZE];
	//read epprom value
	eeprom_readPass(data);
	//check on each element
	for(i=0;i<SIZE;i++){
		if(data[i]>9){//if one element>9 then the epprom is empty and its first usage
			UART_sendByte(EMPTY);
			return EMPTY;
		}
		//if not then check on if pass==data get from epprom
		else if(data[i]==pass[i]){
			//increment counter
			count++;
		}
	}
	if(count==SIZE){//if counter==size the pass is the same send and return same value
		UART_sendByte(SAME);
		return SAME;
	}
	else{//if counter!=size the pass is different send and return different value
		UART_sendByte(DIFFERENT);
		return DIFFERENT;
	}
}
//check on the two pass when the user wants to change the pass
uint8 APP_compareSetPass(uint8 *pass,uint8 *pass2){
	uint8 i,count=0;
	for(i=0;i<SIZE;i++){
		if(pass[i]==pass2[i]){
			count++;
		}
		else{
			break;
		}
	}
	if(count==SIZE){
		UART_sendByte(SAME);
		return SAME;
	}
	else{
		UART_sendByte(DIFFERENT);
		return DIFFERENT;
	}
}

//recieve the pass by uart
void APP_getPass(uint8 *pass){
	uint8 i;
	for(i=0;i<SIZE;i++){
		pass[i]=UART_recieveByte();
	}
}
//initiate the twi prescaler=0, bit rate 400,000 KHz, address 1 if this MCU will be slave
void APP_eppromInit(void){
	TWI_config twi={400000,1,_1};
		TWI_init(&twi);
}
//initiate the uart 8bit frame, even parity, 1 stop bit, rising edge, buard rate=192,000
void APP_uartInit(void){
	UART_config uart={FRAME_8_BITS,PARITY_EVEN,STOP_BITS_1,RISING,19200};
	UART_init(&uart);
}
//write the pass to epprom
void epprom_writePass(uint8 *pass){
	uint8 i,status;
	for(i=0;i<SIZE;i++){
		status=EEPROM_writeByte(0x0001+i,pass[i]);
		_delay_ms(10);
	}

}
//read the pass from epprom
void eeprom_readPass(uint8 *pass){
	sint8 i;
	for(i=0;i<SIZE;i++){
		EEPROM_readByte(0x0001+i,(pass+i));
		_delay_ms(10);
	}

}
