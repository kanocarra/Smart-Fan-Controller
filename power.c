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
 #define GAIN 3.55
 #define CALIBRATION_FACTOR 1.1

 struct PowerController powerController;
 
 static enum Parameters {
	 CURRENT,
	 VOLTAGE,
 };

 enum Parameters calculatedParameter;

//Interrupts 
ISR(TIMER2_COMPB_vect){
	
	if(powerController.cycles < 21){
		// Enable ADC interrupt
		ADCSRA |= (1<<ADIE);
		ADCSRA |= (1<<ADSC);
		powerController.cycles++;
	} else {
		
		switch (calculatedParameter){
			
			case 0: 
			calcRMScurrent();
			switchChannel(calculatedParameter);
			calculatedParameter = VOLTAGE;
			break;

			case 1:
			calcRMSvoltage();
			switchChannel(calculatedParameter);
			calcAveragePower();
			calculatedParameter = CURRENT;
			//Disable ADC Interrupt
			ADCSRA &= ~(1<<ADIE);
			//Stop ADC
			ADCSRA &= ~(1<<ADSC);
			//Disable Timer2 Output Compare Interrupt
			TIMSK2 &= ~(1<<OCIE2B);
			PRR &= ~(1<<PRADC);
			//Clear Registers
			ADCSRA &= ~(ADCSRA);
			ADCSRB &= ~(ADCSRB);
			ADMUXA &= ~(ADMUXA);
			ADMUXB &= ~(ADMUXB);
			DIDR1 &= ~(DIDR1);
			break;

		}
		powerController.numConversions = 0.0;
		powerController.cycles = 0.0;
	}

}

ISR(ADC_vect){

	switch(calculatedParameter){
		
		case CURRENT:
		powerController.current = CALIBRATION_FACTOR * ((float)(ADC) * (V_REF/ADC_RESOLUTION))/R_SHUNT;
		powerController.sqCurrentSum = powerController.sqCurrentSum + pow(powerController.current, 2.0);
		powerController.numConversions++;
		powerController.pulseSample++;
		if(powerController.pulseSample == 3){
			//Disable ADC interrupt
			ADCSRA &= ~(1<<ADIE);
			//Stop ADC
			ADCSRA &= ~(1<<ADSC);
			//Reset the number of pulse samples back to 0
			powerController.pulseSample = 0;
		}
		break;

		case VOLTAGE:
		//calculate original voltage
		powerController.voltage = GAIN * ((float)(ADC) * (V_REF/ADC_RESOLUTION));
		powerController.sqVoltageSum = powerController.sqVoltageSum + pow(powerController.voltage, 2.0);
		powerController.numConversions++;
		powerController.pulseSample++;
		if(powerController.pulseSample == 3){
			//Disable ADC interrupt
			ADCSRA &= ~(1<<ADIE);
			//Stop ADC
			ADCSRA &= ~(1<<ADSC);
			//Reset the number of pulse samples back to 0
			powerController.pulseSample = 0;
		}
		break;
	}

}


 void initialiseADC(void) {
	
	 //Initialize DDRB - Set Port B as inputs
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

	 //Enable Timer2 Output Compare Interrupt
	 TIMSK2 |= (1<<OCIE2B);
	 
	 //Reset all variables
	 powerController.pulseSample = 0;
	 powerController.numConversions = 0.0;
	 powerController.cycles = 0.0;
	 calculatedParameter = CURRENT;
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
	 powerController.RMScurrent = sqrt(powerController.sqCurrentSum/powerController.numConversions);
	 powerController.sqCurrentSum = 0.0;
	 //sendCurrent(power.RMScurrent);
 }

 void calcRMSvoltage(void){

	 powerController.RMSvoltage = sqrt(powerController.sqVoltageSum/powerController.numConversions);
	 powerController.sqVoltageSum = 0.0;
	 //sendVoltage(power.RMSvoltage);
 }

 void calcAveragePower(void){
	 powerController.averagePower = powerController.RMSvoltage * powerController.RMScurrent;
	 powerController.ADCConversionComplete = 1;
	 
 }