/*
 * CommunicationsController.c 
 * Controller for all communications with the master control
 *
 * Created: 5/09/2016 10:32:14 a.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
 */ 

  #define BAUD 9600UL
  #define F_CPU 8000000UL

 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #include <avr/sleep.h>
 #include <avr/wdt.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #define FAN_ID 2
 #define SW_VERSION 1.1
 #define END_PACKET 10
 #define R 82
 #define SPEED_REQUEST 83
 #define STATUS_REQUEST 63
 #define CR 13
 #define TO_ASCII 48

 #include "prototypes.h"

 struct CommunicationsController communicationsController;

  enum ByteReceived {
	  SOURCE_ID,
	  DEST_ID,
	  MESSAGE_ID,
	  DATA0,
	  DATA1,
	  DATA2,
	  LF
  };

  enum ByteReceived commStatus = SOURCE_ID;
  
ISR(USART0_RX_vect){
	//Read received data
	uint8_t rX_data = UDR0;
	switch (communicationsController.index) {
		
		// Read the source ID
		case SOURCE_ID:	
			communicationsController.sourceId = rX_data - TO_ASCII;
			communicationsController.index++;
			break;
			
		// Read the destination ID
		case DEST_ID:
			communicationsController.destinationId = rX_data - TO_ASCII;
			// Checks that the message is addressed to the smart fan otherwise ignores the packet
			if (communicationsController.destinationId == FAN_ID){
				communicationsController.index++;
			} else {
				communicationsController.index = LF;
				communicationsController.destinationId = 0;
			}
			break;
		case MESSAGE_ID:
			// Stores the message ID
			communicationsController.messageId = rX_data;	
			if(communicationsController.messageId == STATUS_REQUEST){
				communicationsController.index = LF;
			} else {
				communicationsController.index++;
			}
			break;

		case DATA0:
			// If a new speed is requested
			if(communicationsController.messageId == SPEED_REQUEST){
					communicationsController.speedValues[communicationsController.speedIndex] =  rX_data - TO_ASCII;
					communicationsController.speedIndex++;
			}
			communicationsController.index++;
			
			break;
		
		case DATA1:
			// Store second speed value
			communicationsController.speedValues[communicationsController.speedIndex] =  rX_data - TO_ASCII;
			communicationsController.speedIndex++;
			communicationsController.index++;
			break;
		
		case DATA2:
			//Store third speed value
			communicationsController.speedValues[communicationsController.speedIndex] =  rX_data - TO_ASCII;
			communicationsController.speedIndex++;
			communicationsController.requestedSpeed = communicationsController.speedValues[0] * 1000 
													+ communicationsController.speedValues[1] * 100 
													+  communicationsController.speedValues[2] * 10;
			communicationsController.index++;
			break;
		
		case LF:
			// Get end of packet character 
			if(rX_data == END_PACKET || rX_data == CR) {
				// If speed requested
				if(communicationsController.messageId == SPEED_REQUEST) {
					communicationsController.requestedSpeed = communicationsController.speedValues[0] * 1000 
															+ communicationsController.speedValues[1] * 100 
															+ communicationsController.speedValues[2] * 10;
					communicationsController.speedValues[0] = 0;
					communicationsController.speedValues[1] = 0;
					communicationsController.speedValues[2] = 0;
					communicationsController.transmissionComplete = 1;
				} else if (communicationsController.messageId == STATUS_REQUEST){
					communicationsController.transmissionComplete = 1;
				}
				communicationsController.index = 0;
			} else {
				communicationsController.index = LF;
			}
			communicationsController.speedIndex = 0;
			communicationsController.destinationId = 0;
		
			break;
		
		default:
			// If an extra character comes in, reset everything
			communicationsController.index = 0;
			communicationsController.speedIndex = 0;
			communicationsController.destinationId = 0;
	}
}


ISR(USART0_START_vect){
	
	// If error status was locked
	if(errorStatus == LOCKED){
		errorStatus = NONE;
		//Turn off watchdog timer
		WDTCSR &= ~(WDTCSR);
		wdt_enable(WDTO_15MS);
		//Wait for watchdog to reset device
		while(1);
	}
	
	// Indicate transmission start
	communicationsController.transmissionStart = 1;
	errorStatus = NONE;

	// Disable receive start interrupt
	UCSR0D &= ~(1<<SFDE0) & ~(1<<RXSIE0);	
}


void initialiseUART()
{	
	//Clear all registers
	UCSR0B &= ~(UCSR0B);
	UCSR0C &= ~(UCSR0C);
	UBRR0 &= ~(UBRR0);

	// Set the UBRR value based on the baud rate and clock frequency 
	unsigned int ubrrValue = ((F_CPU)/(BAUD*16)) - 1;
	communicationsController.index = 0;
	communicationsController.messageId = 0;
	communicationsController.speedIndex = 0;

	// Setting the UBRR0 value using its High and Low registers
	UBRR0H = (ubrrValue>>8);
	UBRR0L = ubrrValue;
	
	// Enabling the USART receiver and transmitter
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0);

	//Enable receive interrupt
	enableReceiver();
	
	// Set frame size to 8-bits
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);

	// Map Tx to PA7 and Rx to PB2
	REMAP |= (1<<U0MAP);
}

void enableStartFrameDetection(void) {
	
	// Enable start frame detection and wake up from sleep modes on RX start
	 UCSR0D |= (1<<SFDE0) | (1<<RXSIE0);
}

void disableReceiver(void){
	// Disable UART receive interrupt
	UCSR0B &= ~(1<<RXCIE0) & ~(1<<RXEN0);
}

void enableReceiver(void) {
	// Enable UART receive interrupt
	UCSR0B |= (1<<RXCIE0) | (1<<RXEN0);
}

void TransmitUART(uint8_t TX_data)
{	
	// Wait for data register to be empty
	while(!(UCSR0A & (1<<UDRE0)));
	
	// Transmit the given data
	UDR0 = TX_data;

	//Wait until transmit complete
	while(!(UCSR0A & (1<<TXC0)));
}

// Flushes the data register
void USART_Flush( void )
{
	unsigned char data;
	while ( UCSR0A & (1<<RXC0) ) data = UDR0;
}

void sendStatusReport(uint16_t requestedSpeed, float currentSpeed, float power, uint8_t error) {
	// Puts header values into the send packet (source id, destination ID and message ID)
	communicationsController.sendPacket[SOURCE_ID] = FAN_ID + TO_ASCII;
	communicationsController.sendPacket[DEST_ID] = (communicationsController.sourceId) + TO_ASCII;
	communicationsController.sendPacket[MESSAGE_ID] = R;
	communicationsController.sendPacketIndex = DATA0;
	
	// Put software version into the header
	convertDecimal(SW_VERSION);
	
	// Put requested and current speed into send packet
	convertToPacket(requestedSpeed);
	convertToPacket((uint16_t)currentSpeed);
	
	//Put power usage into send packet
	convertDecimal(power);
	
	// Put in the error status
	if(error == NONE){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = '-';
	} else if(error == LOCKED){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = 'L';
	} else if (error == BLOCKED){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = 'B';
	}
	
	communicationsController.sendPacketIndex++;
	
	// Put end packet character
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = END_PACKET;
	
	//Send the whole packet over UART
	int i = 0;
	while(i <= communicationsController.sendPacketIndex){
		TransmitUART(communicationsController.sendPacket[i]);
		i++;
	}
	communicationsController.sendPacketIndex = 0;
	
}

void convertToPacket(uint16_t speed){
	unsigned int factor = 10000;
	unsigned int convertNumber = speed;
	
	// Split the number into individual digits for sending
	while(factor>10){
		factor = factor/10;
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = (convertNumber/factor) + TO_ASCII;
		convertNumber = convertNumber % factor;
		communicationsController.sendPacketIndex++;
	}
}

void convertDecimal(float number){
	uint8_t decimal;
	uint8_t interger;
	uint8_t factor = 100;
	
	// Round to the nearest 2 decimal place
	float roundedNumber = roundf(number * 100) / 100;
	
	if(roundedNumber <= 0) {
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = '-';
		communicationsController.sendPacketIndex++;	
		return;	
	}
	
	// Get the integer part
	interger = (uint8_t)(roundedNumber);
	
	//Get the decimal part
	decimal = (100.0) * (roundedNumber - interger);
	
	// Put the integer part into the packet
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = interger + TO_ASCII;
	communicationsController.sendPacketIndex++;
	
	// Put a decimal point
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = '.';
	communicationsController.sendPacketIndex++;
	
	// Split decimal into digits for the send packet
	while(factor>10){
		factor = factor/10;
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = (decimal/factor) + TO_ASCII;
		decimal = decimal % factor;
		communicationsController.sendPacketIndex++;
	}
	
}

void sendError(char errorType){
	// Puts header values into the send packet (source id, destination ID)
	communicationsController.sendPacket[SOURCE_ID] = FAN_ID + TO_ASCII;
	communicationsController.sendPacket[DEST_ID] = 0 + TO_ASCII;
	
	// Puts the error into the packet
	communicationsController.sendPacket[MESSAGE_ID] = (uint8_t)errorType;
	
	// Adds end packet character 
	communicationsController.sendPacket[3] = END_PACKET;
	
	// Send the packet
	int i = 0;
	while(i < 4){
		TransmitUART(communicationsController.sendPacket[i]);
		i++;
	}
}

// Send functions for testing and calibration purposes
void sendSpeedRpm(float averageSpeed){
	uint8_t tx_data = (uint8_t)(averageSpeed/10.0);
	TransmitUART(tx_data);
}

void sendCurrent(float RMScurrent){
	uint8_t tx_data = (uint8_t)(RMScurrent * 100.0);
	TransmitUART(tx_data);
}

void sendVoltage(float RMSvoltage){
	uint8_t tx_data = (uint8_t)(RMSvoltage * 10.0);
	TransmitUART(tx_data);
}


void sendPower(float averagePower){
	uint8_t tx_data = (uint8_t)(averagePower * 10.0);
	TransmitUART(tx_data);
}