/*
 * power.c
 *
 * Created: 5/09/2016 6:03:52 p.m.
 *  Author: emel269
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #include "prototypes.h"

 #define V_REF 5
 #define R_SHUNT	0.33
 #define ADC_RESOLUTION 1024
 #define	ADC_V_CHANNEL 10
 #define ADC_I_CHANNEL 11
 #define VDR_R1 56
 #define VDR_R2 22
 #define VDR_R3 82

 struct powerParameters power;

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

	 //Enable ADC, Enable ADC Interrupt, Enable ADC Auto Trigger Enable, ADC Pre-scaler (divide by 64),
	 ADCSRA |= (1<<ADEN) | (1<<ADATE) | (1<<ADPS1) | (1<<ADPS2);

	 //Start ADC Conversion when Timer0 overflows
	 ADCSRB |= (1<<ADTS2);

	 initialiseADCTimer();

 }

 float getADCValue(uint8_t ADC_channel){

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

	 //calculate gain
	 float gain = (VDR_R2 + VDR_R3)/(VDR_R1 + VDR_R2 + VDR_R3);
	 //calculate supply voltage
	 float ADC_Voltage = getADCValue(ADC_V_CHANNEL);
	 power.voltage = (gain * ((ADC_Voltage * V_REF)/ADC_RESOLUTION)) * 10;

 }

 void getCurrent(void){
	 //Gain Selection (Gain of 20)
	 ADMUXB |= (1<<GSEL0);

	 //calculate shunt current
	 float ADC_ShuntVoltage = getADCValue(ADC_I_CHANNEL);
	 power.current = (ADC_ShuntVoltage * V_REF)/(ADC_RESOLUTION * R_SHUNT) * 1000;

	 //disable gain
	 ADMUXB &= ~(1<<GSEL0);
 }