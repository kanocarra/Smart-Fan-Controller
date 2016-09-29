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

 #define V_REF 5.0
 #define R_SHUNT 0.33
 #define ADC_RESOLUTION 1024.0
 #define ADC_V_CHANNEL 10
 #define ADC_I_CHANNEL 11
 #define VDR_R1 100.0
 #define VDR_R2 56.0

 struct powerParameters power;
 //global variable
 float numConversions = 0.0;
 float timerCycles = 0.0;
 int calculatedParameter = 0; //Nothing Calculated = 0, Current Calculated = 1, Current & Voltage Calculated = 2
 float gain = 3.55; //10.0;//(VDR_R1 + VDR_R2)/(VDR_R2);
 
//Interrupts 
ISR(ADC_vect){

	switch(calculatedParameter){
		
		case 0:
		//calculate current flowing
		power.current = ADC * (V_REF/ADC_RESOLUTION)/R_SHUNT;
		//TransmitUART((uint8_t)power.current);
		power.sqCurrentSum = power.sqCurrentSum + pow(power.current, 2.0);
		numConversions++;
		break;

		case 1:
		//calculate original voltage
		power.voltage = gain * (ADC * (V_REF/ADC_RESOLUTION));
		power.sqVoltageSum = power.sqVoltageSum + pow(power.voltage, 2.0);
		numConversions++;
		break;

	}

}

ISR(TIMER0_OVF_vect){
	
	if(timerCycles < 20){
		timerCycles++;
	}else{
		//Disable ADC interrupts
		ADCSRA &= ~(1<<ADIE);
		//Stop ADC
		ADCSRA &= ~(1<<ADSC);

		switch(calculatedParameter){

			case 0:
			calcRMScurrent();
			switchChannel(calculatedParameter);
			calculatedParameter = 1;
			break;

			case 1:
			calcRMSvoltage();
			switchChannel(calculatedParameter);
			calculatedParameter = 2;
			break;

			case 2:
			calcAveragePower();
			calculatedParameter = 0;
			break;

		}

	numConversions = 0.0;
	timerCycles = 0.0;
	}	

	//Reset Count
	TCNT0 = 200;

	//Enable ADC Interrupt and start ADC
	ADCSRA |= (1<<ADIE);
	ADCSRA |= (1<<ADSC);

}

 void initialiseADC(void) {

	 //Initialise DDRB - Set Port B as inputs
	 DDRB &= ~(1<<PORTB0) & ~(1<<PORTB3);

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

	 //Initial Channel - Current Sensing
	 ADMUXA = ADC_I_CHANNEL;

	 //Gain Selection (Gain of 20)
	 ADMUXB |= (1<<GSEL0);

	 //Enable ADC, ADC Pre-scaler (divide by 8), Auto Trigger Source, Enable ADC Interrupt
	 ADCSRA |= (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADATE) | (1<<ADIE);

	 initialiseADCTimer();

	 //Start ADC
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

void calcRMScurrent(void){
	
	power.RMScurrent = sqrt(power.sqCurrentSum/numConversions);
	//sendCurrent(power.RMScurrent);
	power.sqCurrentSum = 0.0;

}

void calcRMSvoltage(void){

	 power.RMSvoltage = sqrt(power.sqVoltageSum/numConversions);
	 //sendVoltage(power.RMSvoltage);
	 power.sqVoltageSum = 0.0;
}

void calcAveragePower(void){

	power.averagePower = (power.RMSvoltage * power.RMScurrent);
	sendPower(power.averagePower);

}


void switchChannel(int currentChannel){
	
	switch(currentChannel){
		
		//Switch to voltage sense channel
		case 0:
		//Gain Selection (Gain of 1)
		ADMUXB &= ~(1<<GSEL0);
		//Change ADC Channel
		ADMUXA = ADC_V_CHANNEL;
		break;

		//Switch to current sense channel
		case 1:
		//Gain Selection (Gain of 20)
		ADMUXB |= (1<<GSEL0);
		//Change ADC Channel
		ADMUXA = ADC_I_CHANNEL;
		break;
	}

}