/*
 * Smart Fan Controller
 *
 * Created: 15/08/2016 4:11:57 p.m.
 * Author : emel269
 */ 

 #define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
 
#include "prototypes.h"
#include "state.h"
#include "error.h"


#define F_PWM 18000UL
extern struct pwmParameters pwm;
extern struct speedParameters speedControl;
extern struct powerParameters power;
extern struct communicationsPacket packet;

int main(void)	
{	
	
	State currentState = start;
	//enable global interrupts
	sei();
	
	while (1) {	
		currentState = (State)currentState();
	}
}

State idle(){
	//TransmitUART(power.RMScurrent);
	return (State)idle;
}

State receiveData(){
	return (State)receiveData;
}

State start(){
	initialisePWM(F_PWM, 0.65, 1);
	intialiseSpeedTimer();
	initialiseUART();
	_delay_ms(1000);

	intialiseBlockedDuct();

	intialiseLockedRotor();
	//initialiseADC();
	

	return (State)idle;
}

State changeDirection(){
	return (State)changeDirection;
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
