/*
 * speed.c
 *
 * Created: 5/09/2016 5:34:01 p.m.
 *  Author: emel269
 */ 
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>

 #define F_CPU 8000000UL
 #include "prototypes.h"
 #include "error.h"
 struct speedParameters speedControl;



 ISR(TIMER1_CAPT_vect){
	uint16_t count;

	if(speedControl.lockedRotorDection) {
		count = speedControl.lockedRotorCount;
	} else {
		count = 0;
	}

	speedControl.timerCount = ICR1 -count;
	// speedControl.timerCount = ICR1;
	 speedControl.sampleCounter += speedControl.timerCount;
	 pidController();
	 ICR1 = 0;
	 TCNT1 =  count;
	
 }

 ISR(TIMER1_OVF_vect){

	//Disable interrupt enable on Hall effect 
	ACSR0A &= ~(1<<ACIE0);

	//Send error status 
	errorStatus = LOCKED;
	
	//Disable PWM Channel on TOCC3
	TOCPMCOE &= ~(1<<TOCC3OE);
	//Disable PWM Channel on TOCC5
	TOCPMCOE &= ~(1<<TOCC5OE);
	// Stop timer 1
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);

 }


 void intialiseSpeedTimer(void){

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

	 speedControl.sampleTime = 0;
	 speedControl.lastError = 0;
	 speedControl.lastSpeed = 0;
	 speedControl.prescaler = 64;
	 speedControl.lockedRotorDection = 0;
 }

 void pidController(void){

	float mechanicalFrequency = (uint8_t)((F_CPU/speedControl.prescaler)/speedControl.timerCount);
	speedControl.currentSpeed = ((mechanicalFrequency * 60)/3);

	 if(speedControl.currentIndex < 10) {
		 speedControl.samples[speedControl.currentIndex] = speedControl.currentSpeed;
		 speedControl.currentIndex++;
	 } else {
	   	 calculateAverageRpm();
		 speedControl.sampleTime = speedControl.sampleCounter/(F_CPU/speedControl.prescaler);
		 setSpeed();
		 speedControl.sampleCounter = 0;
		 speedControl.currentIndex = 0;
	 }
 }

 // Calculates the average RPM and clears the speed sample array
 void calculateAverageRpm(void){
	 
	 float sum = 0;
	 float prevSpeed = speedControl.samples[0];
	 int valueCount = 0;

	 for(int i =0; i < speedControl.currentIndex; i++){
		 // Check that the value is not an out-lier (50% larger than previous reading)
		 if(prevSpeed * 1.5 >= speedControl.samples[i]){
			 //Discard the value
			 speedControl.samples[i] = 0;
			 prevSpeed = speedControl.samples[i];
			 } else {
			 // Add to the total sum
			 sum = sum + speedControl.samples[i];
			 prevSpeed = speedControl.samples[i];
			 speedControl.samples[i] = 0;
			 valueCount++;
		 }
	 }
	 speedControl.averageSpeed  = sum/valueCount;
 }

 void setSpeed(void){
	 float kP = 0.0625;
	 float kI = 0.003;
	 float kD = 0.08;
	 float proportionalGain;
	 float output;

	 //Max PWM Output
	 double Max = 150;
	 double Min = 0;
	
	 float error = speedControl.requestedSpeed - speedControl.currentSpeed;
	 
	 //if(error < -700){
		 //error = -200;
	 //}
	 
	 speedControl.errorSum = (speedControl.errorSum + error) * speedControl.sampleTime;

	 //clamp the integral term between 0 and 400 to prevent integral windup
	 if(speedControl.errorSum > Max) speedControl.errorSum = Max;
	 else if(speedControl.errorSum < Min) speedControl.errorSum = Min;

	 output = kP * error + (kI * speedControl.errorSum) - (kD * (speedControl.currentSpeed - speedControl.lastSpeed)/speedControl.sampleTime); 
	 
	 //clamp the outputs between 0 and 400 to prevent windup
	 //if(output> max) output = max;
	 //else if(output < min) output = min;

	 speedControl.lastError = error;
	 speedControl.lastSpeed = speedControl.currentSpeed;

	 proportionalGain = speedControl.requestedSpeed/(speedControl.requestedSpeed - output);
	 setDutyCycle(proportionalGain);
	 
 }

 void setRequestedSpeed(unsigned int speed){
	
	// Changes requested speed
	speedControl.requestedSpeed = speed;
	
	// Reset errors for controller
	speedControl.lastError = 0;
	speedControl.errorSum = 0;
	setSpeed();
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
