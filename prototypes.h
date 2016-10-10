/*
 * smart_fan_prototypes.h
 *
 * Created: 5/09/2016 10:47:00 a.m.
 *  Author: emel269
 */ 


#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

enum Errors {
	NONE,
	LOCKED,
	BLOCKED
};

enum Errors errorStatus;

struct PwmController {
	float dutyCycle;
	uint16_t top;
};

struct SpeedController{
	float samples[10];
	int currentIndex;
	uint16_t requestedSpeed;
	float currentSpeed;
	float averageSpeed;
	uint16_t timerCount;
	float errorSum;
	float lastError;
	float sampleTime;
	float sampleCounter;
	float lastSpeed;
	uint16_t lockedRotorCount;
	uint8_t prescaler;
	uint8_t lockedRotorDection;
	uint8_t isCalibrated;
	uint8_t blockedCount;
};

struct PowerController{
	float current;
	float voltage;
	float sqCurrentSum;
	float sqVoltageSum;
	float RMScurrent;
	float RMSvoltage;
	float averagePower;
	uint8_t ADCConversionComplete;
	int pulseSample;
	uint16_t numConversions;
	uint16_t cycles;
};

struct CommunicationsController {
	uint8_t index;
	uint8_t sourceId;
	uint8_t destinationId;
	uint8_t messageId;
	uint8_t speedValues[3];
	uint8_t speedIndex;
	uint8_t transmissionComplete;
	uint8_t sendPacket[17];
	uint8_t sendPacketIndex;
	uint16_t requestedSpeed; 
	uint8_t transmissionStart;
};

/*************************** PWM GENERATION **************************/
/********************************************************************/

void initialisePwmController(float dutyCycle, uint8_t pin);

// Hall Effect sensor
void initialiseAnalogComparator(void);

//PWM waveform generation
void initialisePWMtimer(uint8_t pin);

// Adjust the PWM duty cycle
void setDutyCycle(float gain);

//Stops the fan
 void stopFan(void);


/************************* FAN SPEED CONTROL ************************/
/*******************************************************************/

// Initialize timer to measure fan speed
void initialiseSpeedController(void);

// Calculate fan speed in rpm
void pidController(void);

// Calculate average fan speed in rpm
void calculateAverageRpm(void);

// Send the RPM value/10
void sendSpeedRpm(float averageSpeed);

// Set the new speed
void setSpeed(void);

// Set a new requested speed
void setRequestedSpeed(uint16_t speed);

//Initialize Locked Rotor 
void intialiseLockedRotor(void);


/******************** POWER CONSUMPTION MEASUREMENT *****************/
/*******************************************************************/

// Initialiase Analog to digital converter
void initialiseADC(void);

// Calculate voltage
void calcRMSvoltage(void);

// Calculate RMS current
void calcRMScurrent(void);

//Calculate Average Power
void calcAveragePower(void);

//Switches the ADC input channel and gain accordingly
void switchChannel(int currentChannel);

//Send the RMS current value
void sendCurrent(float RMScurrent);

//Send the RMS voltage value
void sendVoltage(float RMSvoltage);

//Send the average power value
void sendPower(float averagePower);



/**************************** COMMUNICATIONS ************************/
/*******************************************************************/

// Initialising UART 
void initialiseUART();

// Transmit the data over UART
void TransmitUART(uint8_t TX_data);

void sendStatusReport(uint16_t requestedSpeed, float speed, float power,uint8_t error);

void disableReceiver(void);

void enableReceiver(void);

void disableTransmitter(void);

void enableTransmitter(void);

void convertToPacket(uint16_t speed);

void sendError(char errorType);

void enableStartFrameDetection(void);

 void USART_Flush( void );

/**************************** BLOCKED DUCT CALIBRATION ************************/
/*******************************************************************/

//calibrate the power corresponding at different speeds
void intialiseBlockedDuct(void);

//check if the duct is blocked
 uint8_t checkBlockDuct(float speed);
 
 
 // General functions
 
 void initialiseWatchDogTimer(void);

 void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

 void turnOffWatchDogTimer(void);

 void convertDecimal(float number);

#endif /* PROTOTYPES_H_ */