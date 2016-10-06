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
#include <avr/wdt.h>
 
#include "prototypes.h"
#include "state.h"

#define F_PWM 18000UL
#define SPEED_REQUEST 83
#define STATUS_REQUEST 63
extern struct pwmParameters pwm;
extern struct speedParameters speedControl;
extern struct powerParameters power;
extern struct communicationsPacket packet;

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

int main(void)	
{	
	State currentState = sleep;
	errorStatus = NONE;
	speedControl.currentSpeed = 0;
	speedControl.requestedSpeed = 0;
	packet.transmissionStart = 0;
	
	initialiseUART();
	enableStartFrameDetection();
	
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

			if(speedControl.requestedSpeed <= 0){
				return (State)sleep;
			} else if(speedControl.currentSpeed == 0){
				return (State)start;
			}

			packet.transmissionStart = 0;
			break;
			//
		case STATUS_REQUEST:
			//Disables UART until speed has been changed
			disableUART();

			_delay_ms(100);

			sendStatusReport(speedControl.requestedSpeed, speedControl.currentSpeed,  power.averagePower, errorStatus);
			
			// Reset transmission for a new frame
			packet.transmissionComplete = 0;
				
			// Reset message ID
			packet.messageId = 0;
			
			//Clear transmission start
			packet.transmissionStart = 0;
			
			_delay_ms(100);
			
			// Re-enable UART
			enableUART();

			break;
		
		default:
			// Set transmission as not complete
			packet.transmissionComplete = 0;
			// Reset message ID
			packet.messageId = 0;
			//Clear transmission start
			packet.transmissionStart = 0;
	}
	return (State)controlSpeed;
}


State start(){
	initialisePWM(F_PWM, 0.65, 1);
	intialiseSpeedTimer();
	//initialiseADC();

	_delay_ms(1000);
	//intialiseBlockedDuct();

	intialiseLockedRotor();
	//initialiseADC();
	return (State)controlSpeed;
}

void wdt_init(void)
{
	MCUSR = 0;
	wdt_disable();

	return;
}

State changeDirection(){
	return (State)changeDirection;
}

State controlSpeed(){
	
	if(errorStatus == LOCKED) {
		return (State)fanLocked;
	} else if(errorStatus == BLOCKED) {
		return (State)blockedDuct;	 	 
	} else if(packet.transmissionComplete) {
			return (State)receiveData;
	} else {
		return(State)controlSpeed;
	}
}

State fanLocked(){
	
	// Send error
	sendError('L');
	
	_delay_ms(100);


	// Transmission clear
	packet.transmissionStart = 0;
	speedControl.currentSpeed = 0;
	speedControl.requestedSpeed = 0;
	packet.transmissionStart = 0;
	
	//initialiseWatchDogTimer();
	enableStartFrameDetection();

	wdt_init();
	wdt_enable(WDTO_1S);

	return (State)sleep;
	
} 

State blockedDuct(){
	return (State)controlSpeed;
}

State sleep(){
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	if (!packet.transmissionStart) {
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
		//Disable start packet interrupt
		UCSR0D &= ~(1<<SFDE0) & ~(1<<RXSIE0);
	}
	//Enable global interrupts
	sei();
	return (State)idle;
}