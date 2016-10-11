/*
 * prototypes.h
 * Headers for all functions and controllers 
 *
 * Created: 5/09/2016 10:47:00 a.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
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
	uint16_t requestedSpeed;
	float currentSpeed;
	uint8_t currentIndex;
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
	uint8_t sendPacket[18];
	uint8_t sendPacketIndex;
	uint16_t requestedSpeed; 
	uint8_t transmissionStart;
};

/*************************** PWM GENERATION **************************/
/********************************************************************/

// Initialises PWM with gien duty cycle
void initialisePwmController(float dutyCycle, uint8_t pin);

// Hall Effect sensor
void initialiseAnalogComparator(void);

//PWM waveform generation
void initialisePWMtimer(uint8_t pin);

// Adjust the PWM duty cycle
void setDutyCycle(float gain);

//Stops the fan
 void stopFan(void);


/************************* SPEED CONTROLLER ************************/
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


/******************** POWER CONTROLLER *****************/
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


/**************************** COMMUNICATIONS CONTROLLER *************/
/*******************************************************************/

// Initialising UART 
void initialiseUART();

// Transmit the data over UART
void TransmitUART(uint8_t TX_data);

//Send the status report
void sendStatusReport(uint16_t requestedSpeed, float speed, float power,uint8_t error);

//Disable the receiver and receive interrupt
void disableReceiver(void);

// Enable receiver and receive interrupt
void enableReceiver(void);

// Converts speed into packet for comms protocol
void convertToPacket(uint16_t speed);

// Converts float into packet for comms protocol
void convertDecimal(float number);

// Send the error state
void sendError(char errorType);

// Enable the receive start interrupt
void enableStartFrameDetection(void);

// Flush the UART
void USART_Flush( void );


/**************************** ERROR CONTROLLER *************************/
/**********************************************************************/

//Check if the duct is blocked
 uint8_t checkBlockDuct(float speed);
 
 //Initialize Locked Rotor detection
 void intialiseLockedRotor(void);
 

/**************************** GENERAL FUNCTIONS ************************/
/**********************************************************************/
 
 // Initialises the watch dog timer
 void initialiseWatchDogTimer(void);

// Makes sure wacthdog doesn't constantly reset program
 void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


#endif /* PROTOTYPES_H_ */