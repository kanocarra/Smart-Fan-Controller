/*
 * Smart Fan Controller
 *
 * Created: 15/08/2016 4:11:57 p.m.
 * Author : emel269
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "prototypes.h"
#include "state.h"
#include "error.h"

#define F_CPU 8000000UL
#define F_PWM 18000UL
extern struct pwmParameters pwm;
extern struct speedParameters speedControl;
extern struct powerParameters power;
extern struct communicationsPacket packet;

int main(void)	
{	
	
	//choose state
	State currentState = start;

	//initialise start motor variables
	turnMotorOff();
	speedControl.isStartState = 1;
	pwm.pinPwm = 1;

	//enable global interrupts
	sei();
	
	while (1) {	
		currentState = (State)currentState();
	}
}

State idle(){
	return (State)idle;
}

State receiveData(){
	return (State)receiveData;
}

State start(){

	if(getMotorState()){
		//select pin 13 as output
		DDRA |= (1<<PORTA6);

		//check to see if motor is on
		if(!(speedControl.isMotorOn)){
		
			//apply DC to 1x coil
			PORTA |= (1<<PORTA6);

			//start timer
			initialiseStartMotorTimer();
		
			//Delay for about 4 seconds
			delaySeconds(4);

			//Turn motor on
			turnMotorOn();

			//turn DC off
			PORTA &= ~(1<<PORTA6);
		}
		//set PWM up
		initialisePWM(F_PWM, 0.75, 1);

		//start PWM signal
		startPWM();

		// **an interrupt should trigger here**

		//return to start state and run the check again
		return (State)start;
	
	}else{
		//START ENTIRE PROGRAME
		//no longer in start state & correct rotation
		initialisePWM(F_PWM, 0.75, 1);

		//start PWM signal
		startPWM();

		//start speed timer
		intialiseSpeedTimer();

		//start UART communication
		initialiseUART();

		//start ADC 
		//initialiseADC();


		return (State)idle;
	}
	
}

State changeDirection(){
	return (State)start;
}

State adjustSpeed(){
	return (State)controlSpeed;
}

State controlSpeed(){
	return (State)controlSpeed;
}

State fanLocked(){
	return (State)sendStatus;
}

State blockedDuct(){
	return (State)sendStatus;

}

State sendStatus(){
	return (State)controlSpeed;
}
