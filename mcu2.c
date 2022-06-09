/*
 * mcu2.c
 *
 *  Created on: Jun 6, 2022
 *      Author: ahmed
 */
#include"app.h"

int main(void){
	//initiate the application
	APP_init();
	while(1){
		//always display the second screen and wait for the selected function to be done
			APP_sndScreen();
}
}

