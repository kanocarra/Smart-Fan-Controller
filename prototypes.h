/*
 * smart_fan_prototypes.h
 *
 * Created: 5/09/2016 10:47:00 a.m.
 *  Author: emel269
 */ 

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_


/*************************** PWM GENERATION **************************/
/********************************************************************/

// Hall Effect sensor
void initialiseAnalogComparator(void);

//PWM waveform generation
void initialisePWMtimer(void);



/************************* FAN SPEED CONTROL ************************/
/*******************************************************************/

// Initialise timer to measure fan speed
void intialiseSpeedTimer(void);

// Calculate fan speed in rpm
void calculateSpeed(uint16_t speedTimerCount);

// Calculate average fan speed in rpm
float calculateAverageRpm(void);

// Send the RPM value/10
void sendSpeedRpm(float averageSpeed);

// Set the new speed
void setSpeed(float actualSpeed);

// Adjust the PWM duty cycle
void changeDutyCycle(void);



/******************** POWER CONSUMPTION MEASUREMENT *****************/
/*******************************************************************/

// Initialiase Analog to digital converter
void initialiseADC(void);

// Initialise timer for ADC
void initialiseADCTimer(void);

// Get the reading from the ADC
float getADCValue(uint8_t ADC_channel);

// Calculate voltage
void getVoltage(void);

// Calculate current
void getCurrent(void);



/**************************** COMMUNICATIONS ************************/
/*******************************************************************/

// Initialising UART 
void InitialiseUART();

// Transmit the data over UART
void TransmitUART(uint8_t TX_data);


#endif /* PROTOTYPES_H_ */