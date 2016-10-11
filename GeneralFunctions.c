/*
 * General Functions.c 
 * Hosts general functions not specific to a controller - includes watchdog timer for sleep states
 *
 * Created: 1/10/2016 12:19:00 p.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/wdt.h>
#define F_CPU 8000000UL
#define F_PWM 18000UL
#include "prototypes.h"

// If the watchdog timer wakes-up the micro-controller
ISR(WDT_vect) {
	if(communicationsController.transmissionStart) {
		errorStatus = NONE;
		} else {
		errorStatus = LOCKED;
	}
}

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

void wdt_init(void)
{
	// Watchdog timer clear and disable at the beginning of the program
	MCUSR = 0;
	wdt_disable();
	return;
}