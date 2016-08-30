/*
 * Smart Fan Controller
 *
 * Created: 15/08/2016 4:11:57 p.m.
 * Author : emel269
 */ 

#include <avr/io.h>
#define F_CPU 8000000
#define F_PWM 18000
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h> 


#include "state.h"
#include "error.h"
#define F_CPU 8000000

//function declarations
void initialiseAnalogComparator(void);
void initialisePWMtimer(void);
void calculateSpeed(int speedTimerCount);
void intialiseSpeedTimer(void);
void initialiseADC(void);

int poleCount = 0;
double dutyCycle = 0.7;

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

	poleCount++;

	if (poleCount == 6){
		int speedTimerCount;
		/// WAIT until capture is triggered
		//while(~(TIFR1 & (1<<ICF1)));
		
		//Capture the count value stored in Input Capture Register
		speedTimerCount = ICR1;
		intialiseSpeedTimer();
		printf("Time = %i", speedTimerCount);
		//calculateSpeed(speedTimerCount);
		poleCount = 0;
		
	}

	//Toggle between rising and falling 
	TCCR1B ^= (1<<ICES1);

}

ISR()
{



}

int main(void)	
{
	// PIN B0 B1 is output 2x LED's
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);

	DDRB |
	
	//initialize PWM timer
	initialisePWMtimer();

	printf("Hello World");
	//initialise timer to calculate speed
	intialiseSpeedTimer();

	//initialize Analog Comparator
	initialiseAnalogComparator();

	//initialise ADC
	initialiseADC();

	//clear port B
	PORTB &= ~(PORTB);

	//enable global interrupts
	sei();

	//State currentState = idle;

	while (1) {	
		//currentState = (State)currentState();
	}
}

void initialiseAnalogComparator(void){

	// clear control and status register A
	 ACSR0A &= ~(ACSR0A);

	// clear control and status register B
	 ACSR0B &= ~(ACSR0B);

	//Set hysteresis level of 50mV
	ACSR0B |= (1<<HSEL0) | (1<<HLEV0);

	//enable output comparator
	ACSR0B |= (1<<ACOE0);

	//set rising edge and input capture enable 
	ACSR0A |= (1<<ACIS01) | (1<<ACIS00) | (1<<ACIC0);
	
	//initialise interrupt enable
	ACSR0A |= (1<<ACIE0);
}

void intialiseSpeedTimer(void){

	//Ensure counter is stopped
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);

	//Disable all Timer1 Interrupt
	TIMSK1 &= ~(1<<ICIE1) & ~(TOIE0);

	//input capture enable
	TCCR1B |= (1<<ICES1);

	//Clearing input capture flag
	TIFR1 |= (1<<ICF1);
		
	//Reset count
	TCNT1 = 0x0000;

	//Start the timer with no prescaler 
	TCCR1B |= (1<< CS10);
	
	}

void initialisePWMtimer(void){

	//initialize variables
	unsigned int prescaler = 1;
	uint16_t top = (F_CPU/(prescaler*F_PWM)) - 1;
	uint16_t compareCount = dutyCycle*top;
	
	//configure data direction register channel 0 as output "PA2" - port A1
	DDRA |= (1<<PORTA4);
	DDRA |= (1<<PORTA6);

	//set compare value
	OCR2A = compareCount;		//necessary for 8 bit number

	//defined TOP value for "WGM 1110"
	ICR2 = top;
	
	//clear registers in charge of set points
	TCCR2A &= ~(TCCR2A);
	TCCR2B &= ~(TCCR2B);

	//Compare Output Mode, Fast PWM
	TCCR2A |= (1<<COM2A1) | (1<<WGM21);
	TCCR2B |= (1<<WGM22) | (1<<WGM23);

	//timer/counter output compare mux TOCC1
	TOCPMSA0 |= (1<<TOCC3S1);
	TOCPMSA1 |= (1<<TOCC5S1);

	// Clear output compare mode channel enable register
	TOCPMCOE &= ~(TOCPMCOE);

	//Enable PWM Channel on TOCC5 first
	TOCPMCOE |= (1<<TOCC5OE);

	//clk pre-scaler = 1 & start timer
	TCCR2B |= (1<<CS20);

}

void calculateSpeed(int speedTimerCount){


}

void initialiseADC(void) {

	//Initialise DDRB
	DDRB |= (0<<PORTB0) | (0<<PORTB3);

	//Clear Registers
	ADCSRA &= ~(ADCSRA);
	ADCSRB &= ~(ADCSRB);
	ADMUXA &= ~(ADMUXA);
	ADMUXB &= ~(ADMUXB);
		
	//Clears Power Reduction Register
	PRR &= ~(1<<PRADC);

	//Disable Digital Input
	DIDR1 |= (1<<ADC11D);

	//Reference Voltage Selection (VCC)
	ADMUXB &= ~(ADMUXB);

	//Gain Selection (Gain of 20)
	ADMUXB |= (1<<GSEL0);

	//Enable ADC, Start ADC Conversion, Enable ADC Interrupt, ADC Pre-scaler (divide by 64), Enable ADC Interrupt
	ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADATE);

	//Start ADC Conversion when Timer0 overflows
	ADCSRB |= (1<<ADTS2);



}

uint16_t getADCValue(uint8_t ADC_channel){

//Clearing the register to select right channel
ADMUXA &= ~(ADMUXA);

//Reading from ADC Channel 11
//ADMUXA |= (1 << MUX0) | (1 << MUX1) | (1 << MUX3);



}

State idle(){
	return (State)idle;

}

State receiveData(){
	return (State)receiveData;
}

State start(){
	return (State)start;
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