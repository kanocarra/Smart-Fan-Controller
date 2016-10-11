/*
 * SpeedController.c 
 * Controller for speed of the fan - including the PID controller and the measurement of speed
 *
 *  Created: 5/09/2016 5:34:01 p.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>

 #define F_CPU 8000000UL
 #include "prototypes.h"
 struct SpeedController speedControl;

 ISR(TIMER1_CAPT_vect){
	uint16_t count;

	if(speedControl.lockedRotorDection) {
		count = speedControl.lockedRotorCount;
	} else {
		count = 0;
	}
	
	//
	speedControl.timerCount = ICR1 -count;
	
	 speedControl.sampleCounter += speedControl.timerCount;
	 measureSpeed();
	 ICR1 = 0;
	 TCNT1 =  count;
	
}

 void initialiseSpeedController(void){

	 // Stop timer
	 TCCR1B &= ~(TCCR1B);
	 TIMSK1 &=~(TIMSK1) ;
	 
	 //Reset input capture register
	 ICR1 = 0;

	 //Set input capture on rising edge
	 TCCR1B |= (1<<ICES1);
	 
	 //Enable input capture interrupt
	 TIMSK1 |= ( 1<< ICIE1);

	 //Start timer with prescaler 64
	 TCCR1B |= (1<<CS11) | (1<<CS10);

	 //Clear Error  
	 speedControl.sampleTime = 0;
	 speedControl.lastError = 0;
	 speedControl.lastSpeed = 0;
	 speedControl.prescaler = 64;
	 speedControl.lockedRotorDection = 0;
 }

 void measureSpeed(void){
	
	//Calculate mechanical frequency 
	float mechanicalFrequency = (uint8_t)((F_CPU/speedControl.prescaler)/speedControl.timerCount);
	//Current speed
	speedControl.currentSpeed = ((mechanicalFrequency * 60)/3);
	
	uint8_t sampleSize = 10;
	
	//Calculate sample time every 10 samples 
	 if(speedControl.currentIndex < sampleSize) {
		 speedControl.currentIndex++;
	 } else {
		 speedControl.currentIndex = 0;
		 speedControl.sampleTime = speedControl.sampleCounter/(F_CPU/speedControl.prescaler);
		 pidController();
		 speedControl.sampleCounter = 0;
	}
 }

 void pidController(void){

	 //PID Constants
	 float kP = 0.0625;
	 float kI = 0.003;
	 float kD = 0.08;
	 float proportionalGain;
	 float output;

	 //Max PWM Output
	 double Max = 255;
	 double Min = 0;

	 //Error
	 float error = speedControl.requestedSpeed - speedControl.currentSpeed;
	 
	 //Differential Error
	 float diffError =  (speedControl.currentSpeed - speedControl.lastSpeed)/speedControl.sampleTime;
	 
	 //Integral SumError
	 speedControl.errorSum = (speedControl.errorSum + error) * speedControl.sampleTime;
		 		
	//If is within target speed, check for blocked duct
	if(error < 200 && error > -200) {
		// Checks that it is calibrated
		if(speedControl.isCalibrated){
			if(checkBlockDuct(speedControl.currentSpeed)){
				if(!errorStatus == LOCKED){
					speedControl.blockedCount++;
					if(speedControl.blockedCount > (speedControl.currentSpeed/20)){
						errorStatus = BLOCKED;
						speedControl.blockedCount = 0;
					}
				}
			} else if(errorStatus == BLOCKED) {
				errorStatus = NONE;
			}
		}
	}
	
	 // If error is too large, reduce step size
	 if(error < -700){
		 error = -200;
	 }
	 
	 //clamp the integral term between 0 and 400 to prevent integral windup
	 if(speedControl.errorSum > Max) {    
		 speedControl.errorSum = Max;
	 } else if(speedControl.errorSum < Min) {
		 speedControl.errorSum = Min;
	 }

	 //Compute PID gain
	 output = (kP * error) + (kI * speedControl.errorSum) - (kD *diffError) ; 
	 
	 //Save the last variables for next computation
	 speedControl.lastError = error;
	 speedControl.lastSpeed = speedControl.currentSpeed;

	 //Output proportional gain on the dutyCycle to adjust speed
	 proportionalGain = speedControl.requestedSpeed/(speedControl.requestedSpeed - output);
	 setDutyCycle(proportionalGain);
	 
 }

 void setRequestedSpeed(uint16_t speed){

	// Changes requested speed
	speedControl.requestedSpeed = speed;
	speedControl.blockedCount = 0;
	// Reset errors for controller
	speedControl.lastError = 0;
	speedControl.errorSum = 0;
 }
