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

 void turnMotorOn(){
	speedControl.isMotorOn = 1;
 }

void turnMotorOff(){
	speedControl.isMotorOn = 0;
} 

unsigned int getMotorState(void){
	return speedControl.isStartState; 
}

void changeMotorState(unsigned int state){
	if(state == 0){
		//exit start state
		speedControl.isStartState = 0;
	}else{
		//stay in start state
		speedControl.isStartState = 1;
	}
}


void initialiseStartMotorTimer(void){
	//Ensure counter is stopped
	TCCR0B &= ~(1<<CS02) & ~(1<<CS01) & ~(1<<CS00);

	//Disable Timer0 Overflow Interrupt
	TIMSK0 &= ~(TOIE0);

	//Clearing overflow flag
	TIFR0 |= (1<<TOV0);
	
	//Reset count
	TCNT0 = 0;

	//Start the timer with 1024 prescaler
	TCCR0B |= (1<<CS00) | (1<<CS02);

}

 void initialiseStartMotorComparator(void){

	  // clear control and status register A
	  ACSR0A &= ~(ACSR0A);

	  // clear control and status register B
	  ACSR0B &= ~(ACSR0B);

	  //Set hysteresis level of 50mV
	  ACSR0B |= (1<<HSEL0) | (1<<HLEV0);

	  //set rising edge and input capture enable
	  ACSR0A |= (1<<ACIS01) | (1<<ACIS00);

	  //initialise interrupt enable
	  ACSR0A |= (1<<ACIE0);
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
		 sendSpeedRpm(speedControl.averageSpeed);
		 speedControl.sampleTime = speedControl.sampleCounter/(F_CPU/prescaler);
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
	 double Max = 400;
	 double Min = 0;

	 float error = speedControl.requestedSpeed - speedControl.currentSpeed;

	 speedControl.errorSum = (speedControl.errorSum + error) * speedControl.sampleTime;

	 //clamp the integral term between 0 and 400 to prevent integral windup
	// if(speedControl.errorSum > Max) speedControl.errorSum = Max;
	 //else if(speedControl.errorSum < Min) speedControl.errorSum = Min;

	 output = kP * error + (kI * speedControl.errorSum) - (kD * (speedControl.currentSpeed - speedControl.lastSpeed)/speedControl.sampleTime); 
	 
	 //clamp the outputs between 0 and 400 to prevent windup
	 if(output> Max) speedControl.errorSum = Max;
	 else if(output < Min) speedControl.errorSum = Min;

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

 void delaySeconds(unsigned int time){
 	int i;

	unsigned int delay = 32 * time;
	// Delay for about 4 seconds
 	for(i = 0; i < delay; i++){
	 	while(!(TIFR0 & (1<<TOV0)));
	 	TIFR0 |= (1<<TOV0);
 	}

 }