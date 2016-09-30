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
#define SPEED_REQUEST 83
#define STATUS_REQUEST 63
extern struct pwmParameters pwm;
extern struct speedParameters speedControl;
extern struct powerParameters power;
extern struct communicationsPacket packet;
enum Errors errorStatus = NONE;

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
	if(packet.transmissionComplete){
		return (State)receiveData;
	} else {
		return (State)idle;
	}
	
}

State receiveData(){
	switch(packet.messageId) {
			
		case SPEED_REQUEST:
			//Disables UART until speed has been changed
			disableUART();
			
			//Set the new requested speed
			setRequestedSpeed(packet.requestedSpeed);	
			
			// Reset transmission for a new frame
			packet.transmissionComplete = 0;
			
			// Reset message ID
			packet.messageId = 0;
			
			// Re-enable UART
			enableUART();

			break;
			//
		case STATUS_REQUEST:
			//Disables UART until speed has been changed
			disableUART();

			float sendPower = 1;
			unsigned int error = errorStatus;
			sendStatusReport(speedControl.requestedSpeed, speedControl.currentSpeed,  sendPower, error);
			
			// Reset transmission for a new frame
			packet.transmissionComplete = 0;
				
			// Reset message ID
			packet.messageId = 0;
			
			// Re-enable UART
			enableUART();

			break;
		default:
			// Set transmission as not complete
			packet.transmissionComplete = 0;
			// Reset message ID
			packet.messageId = 0;
	}
	return (State)idle;
}

State start(){
	initialisePWM(F_PWM, 0.65, 1);
	intialiseLockedRotor();
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
