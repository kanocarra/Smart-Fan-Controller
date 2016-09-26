/*
 * pwm.c
 *
 * Created: 5/09/2016 4:15:28 p.m.
 *  Author: emel269
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#define F_CPU 8000000UL
#include "prototypes.h"

struct pwmParameters pwm;

ISR(ANA_COMP0_vect){

	//1 = motor in start state else not
	if(getMotorState()){
		if(TCNT0 >= 195){
			//DIRECTION CORRECT
			//exit start motor procedure
			changeMotorState(0);

			//turn timer 0 off
			TCCR0B &= ~(1<<CS02) & ~(1<<CS01) & ~(1<<CS00);

		}else{
			//DIRECTION INCORRECT
			//turn PWM off
			stopPWM();

			//allow opposite channel to be initialised
			swapStartPinPWM(pwm.pinPwm);

			//re-initialise PWM channels
			initialisePwmTimer(pwm.pinPwm);

			//re-allow DC to motor
			turnMotorOff();

			//stay in start state
			changeMotorState(1);
		}
	 
	}else{
		//NOT IN START STATE
		//disable interrupts
		ACSR0A &= ~(1<<ACIE0);

		//Toggle between PWM channel
		TOCPMCOE ^= (1<<TOCC3OE);
		TOCPMCOE ^= (1<<TOCC5OE);

		//toggle between rising and falling
		ACSR0A ^= (1<<ACIS00);
	
		//enable interrupts once service done
		ACSR0A |= (1<<ACIE0);
	}
}

 void initialisePWM(unsigned long frequency, float dutyCycle, unsigned int prescaler) {
	
	// Set up the PWM parameters
	pwm.frequency = frequency;
	pwm.dutyCycle = dutyCycle;
	pwm.prescaler = prescaler;	

	pwm.top = (F_CPU/(pwm.prescaler*pwm.frequency)) - 1;

	// initialise timer and analog comparator
	initialisePwmTimer(pwm.pinPwm);
	initialiseAnalogComparator();
 }

 void swapStartPinPWM(unsigned int pinPWM){
	
	if(pinPWM == 0){
		pwm.pinPwm = 1;
	}else{
		pwm.pinPwm = 0;
	}

 }

 void stopPWM(void){
	//stop PWM signal
	//reset output compare mux
	TOCPMSA0 &= ~(TOCPMSA0);
	TOCPMSA1 &= ~(TOCPMSA1);

	//reset output channels
	TOCPMCOE &= ~(TOCPMCOE);
	
	//reset/stop clock
	TCCR2B &= ~(TCCR2B);

 }

 void startPWM(void){

	//timer/counter output compare mux TOCC1
	TOCPMSA0 |= (1<<TOCC3S1);
	TOCPMSA1 |= (1<<TOCC5S1);

	//clk pre-scaler = 1 & start timer
	TCCR2B |= (1<<CS20);
 }

 void initialisePwmTimer(unsigned int pinPwm){

	 //Adjust duty cycle
	setDutyCycle(1);
	 
	 //configure data direction register channel 0 as output "PA2" - port A1
	 DDRA |= (1<<PORTA4);
	 DDRA |= (1<<PORTA6);

	 //defined TOP value for "WGM 1110"
	 ICR2 = pwm.top;
	 
	 //clear registers in charge of set points
	 TCCR2A &= ~(TCCR2A);
	 TCCR2B &= ~(TCCR2B);

	 //Compare Output Mode, Fast PWM
	 TCCR2A |= (1<<COM2A1) | (1<<WGM21);
	 TCCR2B |= (1<<WGM22) | (1<<WGM23);

	 // Clear output compare mode channel enable register
	 TOCPMCOE &= ~(TOCPMCOE);

	//pinPWM = 0 for TOCC5
	//pinPWM = 1 for TOCC3
	if(pinPwm == 0){
		
		//enable PWM on TOCC5 first, and disable other
		TOCPMCOE |= (1<<TOCC5OE);
		TOCPMCOE &= ~(1<<TOCC3OE);
	
	}else{
		
		//enable PWM on TOCC3 first, and disable other
		TOCPMCOE |= (1<<TOCC3OE);
		TOCPMCOE &= (1<<TOCC5OE);
	}
}

 void initialiseAnalogComparator(void){

	 // clear control and status register A
	 ACSR0A &= ~(ACSR0A);

	 // clear control and status register B
	 ACSR0B &= ~(ACSR0B);

	 //Set hysteresis level of 50mV
	 ACSR0B |= (1<<HSEL0) | (1<<HLEV0);

	 //set rising edge and input capture enable
	 ACSR0A |= (1<<ACIS01) | (1<<ACIS00) | (1<<ACIC0);

	 //initialise interrupt enable
	 ACSR0A |= (1<<ACIE0);
 }

 void setDutyCycle(float gain) {
	pwm.dutyCycle = gain * pwm.dutyCycle;
	uint16_t compareCount = pwm.dutyCycle*pwm.top;
	OCR2A = compareCount;
 }