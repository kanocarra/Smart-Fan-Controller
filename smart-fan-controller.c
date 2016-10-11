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

#define SPEED_REQUEST 83
#define STATUS_REQUEST 63
extern struct PwmController pwmController;
extern struct SpeedController speedControl;
extern struct PowerController powerController;
extern struct CommunicationsController communicationsController;
enum Errors errorStatus = NONE;

int main(void)	
{	

	State currentState = sleep;
	errorStatus = NONE;
	speedControl.currentSpeed = 0;
	speedControl.requestedSpeed = 0;
	communicationsController.transmissionStart = 0;
	communicationsController.transmissionComplete = 0;
	powerController.averagePower = 0;
	speedControl.isCalibrated = 0;
	
	initialiseUART();
	enableStartFrameDetection();

	if(errorStatus == LOCKED){
		currentState = fanLocked;
	}
	
	sei();

	while(1) {	
		currentState = (State)currentState();
	}
}

State idle() {
	if(communicationsController.transmissionComplete) {
		return (State)receiveData;
	} else {
		return (State)idle;
	}
}

State receiveData(){

	switch(communicationsController.messageId) {
			
		case SPEED_REQUEST:
		
			// Disable receiver until new speed is processed
			disableReceiver();
			
			//Set the new requested speed
			setRequestedSpeed(communicationsController.requestedSpeed);	
			
			// Reset transmission for a new frame
			communicationsController.transmissionComplete = 0;
			
			// Reset message ID
			communicationsController.messageId = 0;
			
			// Re-enable UART
			enableReceiver();
			
			// If speed requested is less than or equal to zero then put fan into sleep state
			if(speedControl.requestedSpeed <= 0){
				WDTCSR &= ~(WDTCSR);
				wdt_enable(WDTO_15MS);
				return (State)sleep;
				
			// If the fan is currently off, go into start state	
			} else if(speedControl.currentSpeed == 0){
				return (State)start;
			}
			
			// Clear transmission start
			communicationsController.transmissionStart = 0; 
			
			break;
		
		case STATUS_REQUEST:

			// Disable receiver for transmission
			disableReceiver();
				
			// Start the ADC to calculate current power
			initialiseADC();
				
			_delay_ms(100);
				
			// Send the status report
			sendStatusReport(speedControl.requestedSpeed, speedControl.currentSpeed, powerController.averagePower, errorStatus);

			// Reset transmission for a new frame
			communicationsController.transmissionComplete = 0;
			
			// Reset message ID
			communicationsController.messageId = 0;
			
			//Clear transmission start
			communicationsController.transmissionStart = 0;
				
			//Flush receiver
			USART_Flush();
				
			_delay_ms(100);
				
			// Re-enable the receiver after transmission
			enableReceiver();
		
			break;
		
		default:
			// Set transmission as not complete
			communicationsController.transmissionComplete = 0;
			// Reset message ID
			communicationsController.messageId = 0;
			//Clear transmission start
			communicationsController.transmissionStart = 0;
	}
	return (State)controlSpeed;
}

State start(){
	
	uint8_t pwmPin = TOCC5OE;
	
	// Initialise PWM with given duty cycle and frequency
	initialisePwmController(0.6, pwmPin);
	
	// Initialise the speed control
	initialiseSpeedController();
	
	_delay_ms(1000);
	
	uint16_t speedPin3 = speedControl.currentSpeed;
	
	cli();
	
	stopFan();
	
	_delay_ms(1000);
	
	sei();
	pwmPin = TOCC3OE;
	
	initialisePwmController(0.6, pwmPin);
	
	// Initialise the speed control
	initialiseSpeedController();
	
	_delay_ms(1000);
	
	uint16_t speedPin5 = speedControl.currentSpeed;
	
	cli();
		
	stopFan();
		
	_delay_ms(1000);
		
	sei();
	
	if(speedPin3 > speedPin5) {
		pwmPin = TOCC3OE;
	}
	
	initialisePwmController(0.6, pwmPin);
		
	// Initialise the speed control
	initialiseSpeedController();
	
	_delay_ms(1000);
	//Initialise the locked and blocked detection
	intialiseBlockedDuct();
	intialiseLockedRotor();
	
	return (State)controlSpeed;
}

State changeDirection(){
	return (State)changeDirection;
}

State controlSpeed(){
	
	if(errorStatus == LOCKED) {
		return (State)fanLocked;
		
	} else if(errorStatus == BLOCKED) {
		return (State)blockedDuct;	 
			 
	} else if(communicationsController.transmissionComplete) {
		return (State)receiveData;
		
    } else {
		return(State)controlSpeed;
	}
}

State fanLocked(){
	
	// Send error for locked
	sendError('L');
	
	_delay_ms(100);

	// Transmission clear
	communicationsController.transmissionStart = 0;
	speedControl.currentSpeed = 0;
	speedControl.requestedSpeed = 0;
	
	// Initialise start frame and watchdog timer for wake up
	enableStartFrameDetection();
	initialiseWatchDogTimer();
	
	// If a new transmission has been started
	if(communicationsController.transmissionStart == 1){
		return (State)idle;
	}
	
	return (State)sleep;

} 

State blockedDuct(){
	
	//Disable reciever for send
	disableReceiver();
	
	// Send error for blocked
	sendError('B');
	
	//Re-enable receiver
	enableReceiver();
	
	// Delay for 1 second
	_delay_ms(1000);

	return (State)controlSpeed;
}

State sleep(){
	
	// Put micro into power down sleep
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	
	// If a tranmission has not started
	if (!communicationsController.transmissionStart) {
		sleep_enable();
		sei();
		sleep_cpu();
		
		//Wake up from sleep
		sleep_disable();
		
		//Disable start packet interrupt
		UCSR0D &= ~(1<<SFDE0) & ~(1<<RXSIE0);
	}
	//Enable global interrupts
	sei();

	// If fan is locked, go into locked state
	if(errorStatus == LOCKED){
		return (State)fanLocked;
	}
	
	// Otherwise go to idle
	return (State)idle;
}
