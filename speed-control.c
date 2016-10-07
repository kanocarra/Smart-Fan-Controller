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
 struct speedParameters speedControl;

 ISR(TIMER1_CAPT_vect){

	 speedControl.timerCount = ICR1;
	 speedControl.sampleCounter += speedControl.timerCount;
	 pidController();
	 ICR1 = 0;
	 TCNT1 = 0;
 }

 void intialiseSpeedTimer(void){

	 // Stop timer
	 TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);
	 
	 //Reset count
	 TCNT1 = 0x0000;

	 //Reset input capture register
	 ICR1 = 0;

	 //Set input capture on rising edge
	 TCCR1B |= (1<<ICES1);
	 
	 // Disable overflow interrupts
	 TIMSK1 &= ~(1<<TOIE1);

	 //Enable input capture interrupt
	 TIMSK1 |= (1<< ICIE1);

	 //Start timer with prescaler 64
	 TCCR1B |= (1<<CS11) | (1<<CS10);

	 speedControl.requestedSpeed = 2000;
	 speedControl.sampleTime = 0;
	 speedControl.lastError = 0;
	 speedControl.lastSpeed = 0;
 }

 void pidController(void){
	unsigned int prescaler = 64;
	float mechanicalFrequency = (uint8_t)((F_CPU/prescaler)/speedControl.timerCount);
	speedControl.currentSpeed = ((mechanicalFrequency * 60)/3);

	 if(speedControl.currentIndex < 10) {
		 speedControl.samples[speedControl.currentIndex] = speedControl.currentSpeed;
		 speedControl.currentIndex++;
	 } else {
	   	 calculateAverageRpm();
		 speedControl.sampleTime = speedControl.sampleCounter/(F_CPU/prescaler);
		 setSpeed();
		 speedControl.sampleCounter = 0;
		 speedControl.currentIndex = 0;


		//check blocked duct
		checkBlockDuct(speedControl.averageSpeed);
		


		 		
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