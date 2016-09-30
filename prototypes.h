/*
 * smart_fan_prototypes.h
 *
 * Created: 5/09/2016 10:47:00 a.m.
 *  Author: emel269
 */ 

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_
struct pwmParameters {
	unsigned long frequency;
	float dutyCycle;
	unsigned int prescaler;
	uint16_t top;
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
	uint16_t lockedRotorCount;
	unsigned int prescaler;
	uint8_t lockedRotorDection;

};

struct powerParameters{
	float voltageSamples[10];
	float currentSamples[10];
	float powerValue;
	float sqCurrentSum;
	float RMScurrent;
	float voltage;
};

struct communicationsPacket {
	uint8_t index;
	uint8_t sourceId;
	uint8_t destinationId;
	uint8_t messageId;
	uint8_t speedValues[3];
	uint8_t speedIndex;
	uint8_t transmissionComplete;
	uint8_t sendPacket[14];
	uint8_t sendPacketIndex;
	unsigned int requestedSpeed; 
};

/*************************** PWM GENERATION **************************/
/********************************************************************/

void initialisePWM(unsigned long frequency, float dutyCycle, unsigned int prescaler);

// Hall Effect sensor
void initialiseAnalogComparator(void);

//PWM waveform generation
void initialisePWMtimer(void);

// Adjust the PWM duty cycle
void setDutyCycle(float gain);


/************************* FAN SPEED CONTROL ************************/
/*******************************************************************/

// Initialize timer to measure fan speed
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

//Initialize Locked Rotor 
void intialiseLockedRotor(void);




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

void sendStatusReport(unsigned int requestedSpeed, float speed, float power, unsigned int error);

void disableUART(void);

void enableUART(void);

void convertToPacket(unsigned int speed);

#endif /* PROTOTYPES_H_ */