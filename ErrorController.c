/*
 * ErrorController.c 
 * Controller for the locked fan and blocked duct error states
 *
 * Created: 1/10/2016 12:19:00 p.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h>
#define F_CPU 8000000UL
#define MAX_SPEED_VALUE 240 
#include "prototypes.h"

extern struct SpeedController speedControl;
extern struct PwmController pwmController;
extern struct CommunicationsController communicationsController;
 
 // If the watchdog timer wakes-up the micro-controller
 ISR(WDT_vect) {
	 if(communicationsController.transmissionStart) {
		 errorStatus = NONE;
		 } else {
		 errorStatus = LOCKED;
	 }
 }
 
 ISR(TIMER1_OVF_vect){

	//Disable interrupt enable on Hall effect
	ACSR0A &= ~(1<<ACIE0);
	
	// Set error status to locked
	errorStatus = LOCKED;
	
	//Disable PWM Channel on TOCC3
	TOCPMCOE &= ~(1<<TOCC3OE);
	
	//Disable PWM Channel on TOCC5
	TOCPMCOE &= ~(1<<TOCC5OE);
	
	// Stop timer 1
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);
 }

uint8_t checkBlockDuct(float speed){

	//polynomial cal
	float a = 3.0 * (pow(10, -12.0));
	float b = -2.0 * (pow(10, -8.0));
	float c = 3.0 * (pow(10, -5.0));
	float d = 0.011;
	float e = 7.2499;
	 
	//polynomial calculations
	float expectedDutyCycle  = a*(pow(speed, 4)) - b*(pow(speed, 3)) + c*(pow(speed, 2)) + d*(speed) + e;
	
	//linear calculations 
	float expectedDutyCyclelinear = 0.0254*(speed) + 7.6481;

		
	if(speed < 350){
		
		return ((pwmController.dutyCycle*100.0) > (1.01*expectedDutyCycle));
	
	}else if(speed > 350 && speed <= 700){

		return ((pwmController.dutyCycle*100.0) > (1.001 * expectedDutyCyclelinear));

	}else if( speed > 700 && speed <= 800){
		return ((pwmController.dutyCycle*100.0) > (1.05 * expectedDutyCyclelinear));
	
	}else if (speed > 800 && speed <= 1250){

		return ((pwmController.dutyCycle*100.0) > (1.1 * expectedDutyCyclelinear));

	}else if(speed > 1250 && speed <= 1550){

		return ((pwmController.dutyCycle*100.0) > (1.13 * expectedDutyCyclelinear));

	}else {

		return ((pwmController.dutyCycle*100.0) > (1.1 * expectedDutyCyclelinear));
	}
}

 void intialiseLockedRotor(void){
	 
	 float cutoffRMP = 100;

	 speedControl.lockedRotorCount =  65535-(uint16_t)(F_CPU/((cutoffRMP*3.0/60.0) * speedControl.prescaler));
	 
	 speedControl.lockedRotorDection = 1;

	 //Set Counter1 Count
	 TCNT1 =  speedControl.lockedRotorCount;
	 // Enable overflow interrupts
	 TIMSK1 |= (1<<TOIE1);
 }
