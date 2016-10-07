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
 int pulseSample = 0;
 float numConversions = 0.0;
 float cycles = 0.0;
 int calculatedParameter = 0; //Calculate Current = 0, Calculate Voltage = 1.
 float gain = 3.55; //(VDR_R1 + VDR_R2)/(VDR_R2);
 
//Interrupts 
ISR(TIMER2_COMPB_vect){
	
	if(cycles < 21){
		// Enable ADC interrupt
		ADCSRA |= (1<<ADIE);
		ADCSRA |= (1<<ADSC);
		cycles++;
	} else {
		
		switch (calculatedParameter){
			
			case 0: 
			calcRMScurrent();
			switchChannel(calculatedParameter);
			calculatedParameter = 1;
			break;

			case 1:
			calcRMSvoltage();
			switchChannel(calculatedParameter);
			calcAveragePower();
			calculatedParameter = 0;
			//Disable Timer2 Output Compare Interrupt
			TIMSK2 &= ~(1<<OCIE2B);
			//Enable Timer0 Overflow Interrupt
			TIMSK0 |= (1<<TOIE0);
			//Start the ADC start interrupt timer with 64 prescaler
			TCCR0B |= (1<<CS01) | (1<<CS00);
			break;

		}
		numConversions = 0.0;
		cycles = 0.0;
		
	}

}

ISR(ADC_vect){

	switch(calculatedParameter){
		
		case 0:
		power.current = ((float)(ADC) * (V_REF/ADC_RESOLUTION))/R_SHUNT;
		power.sqCurrentSum = power.sqCurrentSum + pow(power.current, 2.0);
		numConversions++;
		pulseSample++;
		if(pulseSample == 3){
			//Disable ADC interrupt
			ADCSRA &= ~(1<<ADIE);
			//Stop ADC
			ADCSRA &= ~(1<<ADSC);
			//Reset the number of pulse samples back to 0
			pulseSample = 0;
		}
		break;

		case 1:
		//calculate original voltage
		power.voltage = gain * ((float)(ADC) * (V_REF/ADC_RESOLUTION));
		power.sqVoltageSum = power.sqVoltageSum + pow(power.voltage, 2.0);
		numConversions++;
		pulseSample++;
		if(pulseSample == 3){
			//Disable ADC interrupt
			ADCSRA &= ~(1<<ADIE);
			//Stop ADC
			ADCSRA &= ~(1<<ADSC);
			//Reset the number of pulse samples back to 0
			pulseSample = 0;
		}
		break;

	}

}

ISR(TIMER0_OVF_vect){
	
	//Disable ADC Timer and it's Interrupt Enable.
	TIMSK0 &= ~(1<<TOIE0);
	TCCR0B &= ~(TCCR0B);

	//Clearing overflow flag
	TIFR0 |= (1<<TOV0);

	//Reset Timer count
	TCNT0 = 130;
	
	//Renable Timer2 Output Compare Interrupt
	TIMSK2 |= (1<<OCIE2B);

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
	 
	 //Enable Timer2 Output Compare Interrupt
	 TIMSK2 |= (1<<OCIE2B);

}

void initialiseADCTimer(void){

	//Ensure counter is stopped
	TCCR0B &= ~(TCCR0B);

	//Clearing overflow flag
	TIFR0 |= (1<<TOV0);
		 
	//Inital Count Value
	TCNT0 = 130;

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

 void calcRMScurrent(void){
	 power.RMScurrent = sqrt(power.sqCurrentSum/numConversions);
	 power.sqCurrentSum = 0.0;
	 //sendCurrent(power.RMScurrent);
	 
 }

 void calcRMSvoltage(void){

	 power.RMSvoltage = sqrt(power.sqVoltageSum/numConversions);
	 power.sqVoltageSum = 0.0;
	 //sendVoltage(power.RMSvoltage);
 }

 void calcAveragePower(void){
	 power.averagePower = power.RMSvoltage * power.RMScurrent;
	 //sendPower(power.averagePower);
 }


