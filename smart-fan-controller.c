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

int main(void)	
{	
	
	//initialiseADC();
	State currentState = start;
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
	initialisePWM(F_PWM, 0.75, 1);
	intialiseSpeedTimer();
	initialiseUART();

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
