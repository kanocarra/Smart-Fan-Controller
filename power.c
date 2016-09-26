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
 #define ADC_V_CHANNEL 10 // I thought this one was 9
 #define ADC_I_CHANNEL 11
 #define VDR_R1 56
 #define VDR_R2 22
 #define VDR_R3 82

 struct powerParameters power;
 //global variable
 int numCConversions = 0;
 int timerCycles = 0;
 int ADC_initialised = 1;
 int parameterCalculated = 0; //Nothing Calculated = 0, Current Calculated = 1, Voltage Calculated = 2

//interrupts here
ISR(ADC_vect){

	switch(parameterCalculated){
		
		case 0:
		power.sqCurrentSum = power.sqCurrentSum + pow(ADC, 2);
		numCConversions++;
		break;

		case 1:
		power.sqVoltageSum = power.sqVoltageSum + pow(ADC, 2);
		numCConversions++;
		break;

	}

}

ISR(TIMER0_OVF_vect){
	
	/*if(ADMUXA == ADC_I_CHANNEL){
		getCurrent();
	}else if(ADMUXA == ADC_V_CHANNEL){
		//getVoltage();
	}*/

	
	if(timerCycles < 10){
		timerCycles++;
	}else{
		//Disable ADC interrupts
		ADCSRA &= ~(1<<ADIE);

		switch(parameterCalculated){

			case 0:
			power.RMScurrent = (sqrt(power.sqCurrentSum/numCConversions) * 1000);
			parameterCalculated = 1;
			break;

			case 1:
			power.RMSvoltage = sqrt(power.sqVoltageSum/numCConversions);
			parameterCalculated = 2;
			break;

			case 2:


	}

		
			

		
		sendCurrent(power.RMScurrent);
		numCConversions = 0;
		timerCycles = 0;
		power.sqCurrentSum = 0;
		TCNT0 = 200;
		getCurrent();
	}


}

 void initialiseADC(void) {

	 //Initialise DDRB - Set Port B as inputs
	 DDRB &= ~(1<<PORTB0);
	 DDRB &= ~(1<<PORTB3);

	 //Clears Power Reduction Register
	 PRR &= ~(1<<PRADC);

	 //Clear Registers
     ADCSRA &= ~(ADCSRA);
	 ADCSRB &= ~(ADCSRB); //This also sets ADC to Free Running Mode
	 ADMUXA &= ~(ADMUXA);

	 //Disable Digital Input (Current Sense Input)
	 DIDR1 |= (1<<ADC11D);

	 //Reference Voltage Selection (VCC)
	 ADMUXB &= ~(ADMUXB);

	 //Initial Channel - Current
	 ADMUXA = ADC_I_CHANNEL;

	 //Gain Selection (Gain of 20)
	 ADMUXB |= (1<<GSEL0);

	 //Enable ADC, ADC Pre-scaler (divide by 8), Auto Trigger Source, Enable ADC Interrupt
	 ADCSRA |= (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADATE) | (1<<ADIE);

	 initialiseADCTimer();

	 ADCSRA |= (1<<ADSC);

 }

void initialiseADCTimer(void){

	 //Ensure counter is stopped
	 TCCR0B &= ~(TCCR0B);

	 //Enable Timer0 Overflow Interrupt
	 TIMSK0 |= (1<<TOIE0);

	 //Clearing overflow flag
	 TIFR0 |= (1<<TOV0);
	  
	 //Reset count
	 TCNT0 = 200;

	 //Start the timer with 8 prescaler
	 TCCR0B |= (1<<CS01);

}

void getCurrent(void){
	 


	 //calculate shunt current
	 getADCValue(ADC_I_CHANNEL);
	 //power.current = (ADC_ShuntVoltage * V_REF)/(ADC_RESOLUTION * R_SHUNT) * 1000;

	 //disable gain
	 ADMUXB &= ~(1<<GSEL0);
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

