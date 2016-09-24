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
	
	State currentState = start;
	//enable global interrupts
	sei();
	
	while (1) {	
		currentState = (State)currentState();
	}
}

State idle(){
	//TransmitUART(power.RMScurrent);
	return (State)receiveData();
}

State receiveData(){

	if(packet.transmissionComplete){
		
		switch(packet.messageId) {
			
			case 83:
			
			//Disables UART until speed has been changed
			disableUART();

			//Set the new requested speed
			setRequestedSpeed(packet.requestedSpeed);
			
			// Reset transmission for a new frame
			packet.transmissionComplete = 0;
			
			// Re-enable UART
			enableUART();

			break;
			
			case 63:
			//Disables UART until speed has been changed
			disableUART();

			float sendSpeed = speedControl.currentSpeed;
			float sendPower = power.powerValue;
			unsigned int error = errorStatus;

			sendStatusReport(sendSpeed, sendPower, error);
			
			// Reset transmission for a new frame
			packet.transmissionComplete = 0;
			
			// Re-enable UART
			enableUART();

			break;
			default:
			packet.transmissionComplete = 0;
		}

	}
	return (State)receiveData;
}

State start(){
	initialisePWM(F_PWM, 0.65, 1);
	intialiseSpeedTimer();
	initialiseUART();
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
