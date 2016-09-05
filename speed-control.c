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
	 calculateSpeed();
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
	 
 }

 void calculateSpeed(void){
	 unsigned int prescaler = 64;
	 float mechanicalFrequency = (uint8_t)((F_CPU/prescaler)/speedControl.timerCount);
	 float speedRpm = ((mechanicalFrequency * 60)/3);
	 //TransmitUART(speedRpm/10);
	 //setSpeed(speedRpm);

	 if(speedControl.currentIndex < 10) {
		 speedControl.samples[speedControl.currentIndex] = speedRpm;
		 speedControl.currentIndex++;
		 } else {
		 calculateAverageRpm();
		 sendSpeedRpm(speedControl.currentSpeed);
		 //setSpeed();
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
	 speedControl.currentSpeed  = sum/valueCount;
 }

 void setSpeed(void){
	 float kP = 1;
	 float kI = 0;
	 float kD = 0;
	 float proportionalGain;
	 float output;

	 float error = speedControl.requestedSpeed - speedControl.currentSpeed;
	 speedControl.errorSum = speedControl.errorSum + error;
	 output = kP * error + kI * speedControl.errorSum + kD * error;

	 proportionalGain = speedControl.requestedSpeed/(speedControl.requestedSpeed - output);
	 //pwm.dutyCycle = proportionalGain * pwm.dutyCycle;
	 setDutyCycle(proportionalGain);
 }