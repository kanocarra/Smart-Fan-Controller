/*
 * smart_fan_prototypes.h
 *
 * Created: 5/09/2016 10:47:00 a.m.
 *  Author: emel269
 */ 

#include <stdbool.h>

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_


struct pwmParameters {
	unsigned long frequency;
	float dutyCycle;
	unsigned int prescaler;
	uint16_t top;
	unsigned int pinPwm;
};

struct speedParameters{
	float samples[10];
	int currentIndex;
	unsigned int requestedSpeed;
	float currentSpeed;
	float averageSpeed;
	uint16_t timerCount;
	float errorSum;
	float lastError;
	float sampleTime;
	float sampleCounter;
	float lastSpeed;
	unsigned int isMotorOn;
	unsigned int isStartState;
};

struct powerParameters{
	float voltageSamples[10];
	float currentSamples[10];
	float sqCurrentSum;
	float RMScurrent;
	float voltage;
};

struct communicationsPacket {
	unsigned int characters[8];
	unsigned int index;
	unsigned int messageId;
	unsigned int speedValues[3];
	unsigned int speedIndex;
};

/*************************** PWM GENERATION **************************/
/********************************************************************/

//start PWM signal
void initialisePWM(unsigned long frequency, float dutyCycle, unsigned int prescaler);

// Hall Effect sensor
void initialiseAnalogComparator(void);

//PWM waveform generation
void initialisePwmTimer(unsigned int);

// Adjust the PWM duty cycle
void setDutyCycle(float gain);

//turn PWM off
void stopPWM(void);

//turn PWM on
void startPWM(void);

//change variable for initial high PWM channel
void swapStartPinPWM(unsigned int pinPWM);


/************************* FAN SPEED CONTROL ************************/
/*******************************************************************/

// Initialise timer to measure fan speed
void intialiseSpeedTimer(void);

// Calculate fan speed in rpm
void pidController(void);

// Calculate average fan speed in rpm
void calculateAverageRpm(void);

// Send the RPM value/10
void sendSpeedRpm(float averageSpeed);

// Set the new speed
void setSpeed(void);

// Set a new requested speed
void setRequestedSpeed(unsigned int speed);

//return if the motor is running
bool returnMotorStatus();

//turn the motor on
void turnMotorOn();

//turn the motor off
void turnMotorOff();

//initialise start motor timer
void initialiseStartMotorTimer(void);

//create DC delay
void delaySeconds(unsigned int time);

//initialise start motor comparator (rising edge)
 void initialiseStartMotorComparator(void);

 //get motor state
 unsigned int getMotorState(void);

 //change motor state
 void changeMotorState(unsigned int state);


/******************** POWER CONSUMPTION MEASUREMENT *****************/
/*******************************************************************/

// Initialiase Analog to digital converter
void initialiseADC(void);

// Initialise timer for ADC
void initialiseADCTimer(void);

// Get the reading from the ADC
void getADCValue(uint8_t ADC_channel);

// Calculate voltage
void getVoltage(void);

// Calculate current
void getCurrent(void);

void sendCurrent(float RMScurrent);


/**************************** COMMUNICATIONS ************************/
/*******************************************************************/

// Initialising UART 
void initialiseUART();

// Transmit the data over UART
void TransmitUART(uint8_t TX_data);


#endif /* PROTOTYPES_H_ */