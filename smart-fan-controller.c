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
#include <avr/sleep.h>
 
#include "prototypes.h"
#include "state.h"

#define F_PWM 18000UL
#define SPEED_REQUEST 83
#define STATUS_REQUEST 63
extern struct pwmParameters pwm;
extern struct speedParameters speedControl;
extern struct powerParameters power;
extern struct communicationsPacket packet;

int main(void)	
{	
	
	State currentState = idle;
	errorStatus = NONE;
	speedControl.currentSpeed = 0;
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	//initialiseSleepMode();
	
	//enable global interrupts
	sei();
	
	initialiseUART();
	enableStartFrameDetection();
	
	// Enable sleep
	sleep_enable();
	
	// Sleep state
	sleep_cpu();
	
	//Disable sleep once device has woken up
	//sleep_disable();
	
	while (1) {	
		currentState = (State)currentState();
	}
}

State idle() {
	if(packet.transmissionComplete) {
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
			if(speedControl.currentSpeed == 0){
				return (State)start;
			} else {
				return (State)controlSpeed;
			}
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
	intialiseSpeedTimer();
	_delay_ms(1000);
	intialiseLockedRotor();
	//initialiseADC();
	return (State)controlSpeed();
}

State changeDirection(){
	return (State)changeDirection;
}

State controlSpeed(){
		if(packet.transmissionComplete){
			return (State)receiveData;
			} else if(errorStatus == LOCKED) {
			if(!packet.errorSent){
				return (State)fanLocked;
				} else {
				return (State)idle;
			}
			} else {
			return (State)controlSpeed;
		}
	return (State)controlSpeed;
}

State fanLocked(){
	sendError('L');
	initialiseWatchDogTimer();
	
	// Sleep the micro-controller
	sleep_enable();
	sleep_cpu();
	
	return (State)idle;
} 

State blockedDuct(){
	return (State)sendStatus;
}
