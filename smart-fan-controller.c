/*
 * Smart Fan Controller
 *
 * Created: 15/08/2016 4:11:57 p.m.
 * Author : emel269
 */ 

#include <avr/io.h>
#define F_CPU 8000000UL
#define F_PWM 18000UL
#define BAUD 9600UL
#define V_REF 5
#define R_SHUNT	0.33
#define ADC_RESOLUTION 1024
#define	ADC_V_CHANNEL 10
#define ADC_I_CHANNEL 11
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h> 

#include "state.h"
#include "error.h"

//function declarations
void initialiseAnalogComparator(void);
void initialisePWMtimer(void);
void calculateSpeed(int speedTimerCount);
void intialiseSpeedTimer(void);
void initialiseADCTimer(void);
void initialiseADC(void);
uint16_t getADCValue(uint8_t ADC_channel);
void getVoltage(void);
void getCurrent(void);
void UART_Init(unsigned int UBRR_VAL);
void UART_Transmit(uint8_t TX_data);

int poleCount = 0;
double dutyCycle = 0.7;

//Global Voltage and Current Variables
uint16_t supplyVoltage;
uint16_t shuntCurrent;

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
		//Capture the count value stored in Input Capture Register
		speedTimerCount = ICR1;
		intialiseSpeedTimer();
		//uint8_t tx_data = speedTimerCount % 255;
		//UART_Transmit(tx_data);
		
		//calculateSpeed(speedTimerCount);
		poleCount = 0;
		
	}

	//Toggle between rising and falling 
	TCCR1B ^= (1<<ICES1);
}

int main(void)	
{	
	//initialize PWM timer
	initialisePWMtimer();

	//initialise timer to calculate speed
	intialiseSpeedTimer();

	//initialize Analog Comparator
	initialiseAnalogComparator();

	//initialise ADC and it's timer
	initialiseADCTimer();
	initialiseADC();

	//clear port B
	PORTB &= ~(PORTB);

	//enable global interrupts
	sei();

	unsigned int ubrrValue = ((F_CPU)/(BAUD*16)) - 1;
	UART_Init(ubrrValue);

	getVoltage();
	getCurrent();


	//State currentState = idle;

	//uint16_t tx_data = 65536 - getADCValue(11);
	//uint8_t tx1 = (tx_data>>8);
	//uint8_t tx2 = tx_data;
	//UART_Transmit(tx1);
	//UART_Transmit(tx2);
	
	//supplyVoltage is a 16 bit variable
	UART_Transmit(supplyVoltage);
	//UART_Transmit(shuntCurrent);

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

void initialiseADCTimer(void){

	//Ensure counter is stopped
	TCCR0B &= ~(1<<CS02) & ~(1<<CS01) & ~(1<<CS00);

	//Disable Timer0 Overflow Interrupt
	TIMSK0 &= ~(TOIE0);

	//Clearing input capture flag
	TIFR0 |= (1<<TOV0);
	
	//Reset count
	TCNT0 = 0x0000;

	//Start the timer with 256 prescaler
	TCCR0B |= (1<< CS02);

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

	//Enable ADC, Enable ADC Interrupt, Enable ADC Auto Trigger Enable, ADC Pre-scaler (divide by 64),
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADATE) | (1<<ADPS1) | (1<<ADPS2);

	//Start ADC Conversion when Timer0 overflows
	ADCSRB |= (1<<ADTS2);

}

uint16_t getADCValue(uint8_t ADC_channel){

	//Clearing the register to select right channel
	ADMUXA &= ~(ADMUXA);

	//Reading from ADC_channel
	ADMUXA = ADC_channel;

	//Start ADC Conversion 
	ADCSRA |= (1<<ADSC);

	//Break out of loop only when conversion complete
	while(!(ADCSRA & (1<<ADIF))){}

	//Clear the ADIF control bit
	ADCSRA |= (1<<ADIF);

	return ADC;

}

void getVoltage(void){

	uint16_t ADC_Voltage = getADCValue(ADC_V_CHANNEL);
	supplyVoltage = (ADC_Voltage * V_REF)/ADC_RESOLUTION;

}

void getCurrent(void){
	uint16_t ADC_ShuntVoltage = getADCValue(ADC_I_CHANNEL);
	shuntCurrent = (ADC_ShuntVoltage * V_REF)/(ADC_RESOLUTION * R_SHUNT);
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

void UART_Init(unsigned int UBRR_VAL)
{
	// Setting the UBRR0 value using its High and Low registers
	UBRR0H = (UBRR_VAL>>8);
	UBRR0L = UBRR_VAL;
	
	// Enabling the USART receiver and transmitter
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
	
	// Define what kind of transmission we're using and how many
	// start and stop bits we're using, as well as parity
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);

	REMAP |= (1<<U0MAP);
}

void UART_Transmit(uint8_t TX_data)
{
	// Check that the USART Data Register is empty, AND if UCSR0A
	// is all 0s
	while(!(UCSR0A & (1<<UDRE0)));
	
	// Since UDR is empty put the data we want to send into it,
	// then wait for a second and send the following data
	UDR0 = TX_data;
	
}
