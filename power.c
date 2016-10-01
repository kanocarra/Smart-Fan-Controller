/*
 * power.c
 *
 * Created: 5/09/2016 6:03:52 p.m.
 *  Author: emel269
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #include <math.h>
 #include "prototypes.h"

 #define V_REF 5
 #define R_SHUNT	0.33
 #define ADC_RESOLUTION 1024
 #define ADC_V_CHANNEL 10
 #define ADC_I_CHANNEL 11
 #define VDR_R1 56
 #define VDR_R2 22
 #define VDR_R3 82

 struct powerParameters power;
 //global variable
 int numCConversions = 0;
 int timerCycles = 0;
 int ADC_initialised = 1;

//interrupts here
ISR(ADC_vect){
	power.sqCurrentSum = power.sqCurrentSum + pow(ADC, 2);
	numCConversions++;
}

ISR(TIMER0_OVF_vect){
	
	//start free running
	if(ADC_initialised){
		ADCSRA |= (1<<ADSC);
		ADC_initialised = 0;
		//break;
	}

	
	if(timerCycles < 6){
		timerCycles++;
	}else{	
		//disable ADC interrupts
		ADCSRA &= ~(1<<ADIE);
		power.RMScurrent = (sqrt(power.sqCurrentSum/numCConversions) * 1000);
		sendCurrent(power.RMScurrent);
		numCConversions = 0;
		timerCycles = 0;
		power.sqCurrentSum = 0;
		TCNT0 = 200;
		getCurrent();
	}

}

 void initialiseADCTimer(void){

	 //Ensure counter is stopped
	 TCCR0B &= ~(1<<CS02) & ~(1<<CS01) & ~(1<<CS00);

	 //Disable Timer0 Overflow Interrupt
	 TIMSK0 &= ~(TOIE0);

	 //Clearing overflow flag
	 TIFR0 |= (1<<TOV0);
	 
	 //Reset count
	 TCNT0 = 200;

	 //Start the timer with 8 prescaler
	 TCCR0B |= (1<<CS01);

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

	 //Initial Channel - Current
	  ADMUXA = ADC_I_CHANNEL;

	 //Enable ADC, Enable ADC Interrupt, ADC Pre-scaler (divide by 64),
	 ADCSRA |= (1<<ADEN) | (1<<ADPS1) | (1<<ADPS2) | (1<<ADIE);

	 initialiseADCTimer();

 }

void getADCValue(uint8_t ADC_channel){

	 //Clearing the register to select right channel
	 ADMUXA &= ~(ADMUXA);

	 //Reading from ADC_channel
	 ADMUXA = ADC_channel;

	 //Start ADC Conversion
	 ADCSRA |= (1<<ADSC);

 }

 /*void getVoltage(void){

	 //calculate gain
	 float gain = (VDR_R2 + VDR_R3)/(VDR_R1 + VDR_R2 + VDR_R3);
	 //calculate supply voltage
	 void ADC_Voltage = getADCValue(ADC_V_CHANNEL);
	 power.voltage = (gain * ((ADC_Voltage * V_REF)/ADC_RESOLUTION)) * 10;
 }*/

 void getCurrent(void){
	 //Gain Selection (Gain of 20)
	 ADMUXB |= (1<<GSEL0);

	 //calculate shunt current
	 getADCValue(ADC_I_CHANNEL);
	 //power.current = (ADC_ShuntVoltage * V_REF)/(ADC_RESOLUTION * R_SHUNT) * 1000;

	 //disable gain
	 ADMUXB &= ~(1<<GSEL0);
 }
 
 void initialiseSleepMode(void){
	// Set sleep mode to power-down
	MCUCR |= (1 << SM1);
	 	
	// Turn off USART1
	PRR |= (1<<PRUSART1);
	
 } 