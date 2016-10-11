/*
 * PwmController.c 
 * Controller for PWM control of the fan
 *
 * Created: 5/09/2016 4:15:28 p.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h>
#define F_CPU 8000000UL
#define F_PWM 18000UL
#include "prototypes.h"

struct PwmController pwmController;

ISR(ANA_COMP0_vect)
{
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

 void initialisePwmController(float dutyCycle, uint8_t pin) {
	
	// Set up the PWM parameters
	pwmController.dutyCycle = dutyCycle;
	uint8_t prescaler = 1;	

	pwmController.top = (F_CPU/(prescaler*F_PWM)) - 1;

	// Initialise timer and analog comparator
	initialisePWMtimer(pin);
	initialiseAnalogComparator();
 }

 void initialisePWMtimer(uint8_t pin){

	 //Adjust duty cycle
	setDutyCycle(1);

	 //configure data direction register channel 0 as output "PA2" - port A1
	 DDRA |= (1<<PORTA4);
	 DDRA |= (1<<PORTA6);

	 //defined TOP value for "WGM 1110"
	 ICR2 = pwmController.top;
	 
	 //clear registers in charge of set points
	 TCCR2A &= ~(TCCR2A);
	 TCCR2B &= ~(TCCR2B);

	 //Compare Output Mode, Fast PWM (Inverting Mode)
	 TCCR2A |= (1<<COM2A1) | (1<<WGM21);
	 TCCR2B |= (1<<WGM22) | (1<<WGM23);

	 TOCPMSA0 &= ~(TOCPMSA0);
	 TOCPMSA1 &= ~(TOCPMSA1);

	 //timer/counter output compare mux TOCC1
	 TOCPMSA0 |= (1<<TOCC3S1);
	 TOCPMSA1 |= (1<<TOCC5S1);

	 // Clear output compare mode channel enable register
	 TOCPMCOE &= ~(TOCPMCOE);

	 //Enable PWM Channel on TOCC3 first
	 TOCPMCOE |= (1<<pin);

	 //clk pre-scaler = 1 & start timer
	 TCCR2B |= (1<<CS20);

 }
 
 void stopFan(void) {
	 //Clear PWM registers
	TCCR2A &= ~(TCCR2A);
	TCCR2B &= ~(TCCR2B);
	TOCPMSA0 &= ~(TOCPMSA0);
	TOCPMSA1 &= ~(TOCPMSA1);
	TOCPMCOE &= ~(TOCPMCOE);
	ICR2 = 0;
	
	//Clear speed control registers
	TCCR1B &= ~(TCCR1B);
	TIMSK1 &=~(TIMSK1) ;
	ICR1 = 0;
	
	//Clear analog comparator registers
	ACSR0A &= ~(ACSR0A);
	ACSR0B &= ~(ACSR0B);
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

	 //initialise comparator interrupt enable
	 ACSR0A |= (1<<ACIE0);
 }

 void setDutyCycle(float gain) {
	 
	 // Add the given gain to the PWM duty cycle
	pwmController.dutyCycle = gain * pwmController.dutyCycle;
	
	// If duty cycle tries to go below 13%, then bound it to 13%
	if(pwmController.dutyCycle < 0.13) {
		pwmController.dutyCycle = 0.13;
	}
	
	// Set the new top value for PWM
	uint16_t compareCount = (uint16_t)roundf(pwmController.dutyCycle*(float)pwmController.top);
	OCR2A = compareCount;
	OCR2B = compareCount;
 }

 