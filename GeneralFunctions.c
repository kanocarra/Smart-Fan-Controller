/*
 * GeneralFunctions.c
 *
 * Created: 9/10/2016 11:10:20 PM
 *  Author: kanocarra
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/wdt.h>
#define F_CPU 8000000UL
#define F_PWM 18000UL
#include "prototypes.h"


// Initialize the watchdog timer
void initialiseWatchDogTimer(void){
	
	//Clear watchdog flag
	MCUSR &= ~(1<<WDRF);

	// Write config change protection with watch dog signature
	CCP = 0xD8;

	WDTCSR |= (1<<WDE);

	//Delay of 1s
	WDTCSR |= (1<< WDP1) | (1<<WDP2);

	// Enable watchdog timer interrupt
	WDTCSR |= (1<< WDIE) | (1<< WDE);

}

// Watchdog timer clear at the beginning of the program
void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();
	return;
}

void delaySeconds(unsigned int seconds){
	 int i;

	 unsigned int delay = 32 * seconds;
	 // Delay for about 4 seconds
	 for(i = 0; i < delay; i++){
		 while(!(TIFR0 & (1<<TOV0)));
		 TIFR0 |= (1<<TOV0);
	 }

}