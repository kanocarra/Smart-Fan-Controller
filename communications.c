/*
 * communications.c
 *
 * Created: 5/09/2016 10:32:14 a.m.
 *  Author: emel269
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
	uint8_t rX_data = UDR0;
	switch (communicationsController.index) {
		case SOURCE_ID:	
			communicationsController.sourceId = rX_data - TO_ASCII;
			communicationsController.index++;
			break;

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
			communicationsController.speedValues[communicationsController.speedIndex] =  rX_data - TO_ASCII;
			communicationsController.speedIndex++;
			communicationsController.index++;
			break;
		
		case DATA2:
			communicationsController.speedValues[communicationsController.speedIndex] =  rX_data - TO_ASCII;
			communicationsController.speedIndex++;
			communicationsController.requestedSpeed = communicationsController.speedValues[0] * 1000 
													+ communicationsController.speedValues[1] * 100 
													+  communicationsController.speedValues[2] * 10;
			communicationsController.index++;
			break;
		
		case LF:
			if(rX_data == END_PACKET || rX_data == CR) {
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
			communicationsController.index = 0;
			communicationsController.speedIndex = 0;
			communicationsController.destinationId = 0;
	}
}


ISR(USART0_START_vect){
		
	if(errorStatus == LOCKED){
		errorStatus = NONE;
		//Turn off watchdog timer
		WDTCSR &= ~(WDTCSR);
		wdt_enable(WDTO_15MS);
		//Wait for watchdog to put device to sleep
		while(1);
	}

	communicationsController.transmissionStart = 1;
	errorStatus = NONE;

	// Disable receive start interrupt
	UCSR0D &= ~(1<<SFDE0) & ~(1<<RXSIE0);	
}


void initialiseUART()
{	
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


void TransmitUART(uint8_t TX_data)
{	
	while(!(UCSR0A & (1<<UDRE0)));
	
	// Since UDR is empty put the data we want to send into it,
	// then wait for a second and send the following data
	UDR0 = TX_data;

	//Wait until transmit complete
	while(!(UCSR0A & (1<<TXC0)));
}

void sendStatusReport(uint16_t requestedSpeed, float currentSpeed, float power, uint8_t error) {
	communicationsController.sendPacket[SOURCE_ID] = FAN_ID + TO_ASCII;
	communicationsController.sendPacket[DEST_ID] = (communicationsController.sourceId) + TO_ASCII;
	communicationsController.sendPacket[MESSAGE_ID] = R;
	communicationsController.sendPacketIndex = DATA0;
	convertDecimal(SW_VERSION);
	
	convertToPacket(requestedSpeed);
	convertToPacket((uint16_t)currentSpeed);
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = (uint8_t)(power*10.0) + TO_ASCII;
	communicationsController.sendPacketIndex++;
	
	if(error == NONE){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = '-';
	} else if(error == LOCKED){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = 'L';
	} else if (error == BLOCKED){
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = 'B';
	}
	
	communicationsController.sendPacketIndex++;
	
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = END_PACKET;
	int i = 0;
	while(i <= communicationsController.sendPacketIndex){
		TransmitUART(communicationsController.sendPacket[i]);
		i++;
	}
	communicationsController.sendPacketIndex = 0;
	
}

void disableReceiver(void){
	// Disable UART receive interrupt
	UCSR0B &= ~(1<<RXCIE0) & ~(1<<RXEN0);
}

void enableReceiver(void) {
	// Enable UART receive interrupt
	UCSR0B |= (1<<RXCIE0) | (1<<RXEN0);
}

void convertToPacket(uint16_t speed){
	unsigned int factor = 10000;
	unsigned int convertNumber = speed;

	while(factor>10){
		factor = factor/10;
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = (convertNumber/factor) + TO_ASCII;
		convertNumber = convertNumber % factor;
		communicationsController.sendPacketIndex++;
	}
}

void convertDecimal(float number){
	unsigned int decimal;
	uint8_t interger;
	unsigned int factor = 100;
	// Round to the nearest 2 decimal place
	float roundedNumber = roundf(number * 100) / 100; 

	
	interger = (uint8_t)(roundedNumber);
	decimal = (100.0) * (roundedNumber - interger);
	
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = interger + TO_ASCII;
	communicationsController.sendPacketIndex++;
	
	communicationsController.sendPacket[communicationsController.sendPacketIndex] = '.';
	communicationsController.sendPacketIndex++;
	
	while(factor>10){
		factor = factor/10;
		communicationsController.sendPacket[communicationsController.sendPacketIndex] = (decimal/factor) + TO_ASCII;
		decimal = decimal % factor;
		communicationsController.sendPacketIndex++;
	}
	
}

void sendError(char errorType){
	communicationsController.sendPacket[SOURCE_ID] = FAN_ID + TO_ASCII;
	communicationsController.sendPacket[DEST_ID] = 0 + TO_ASCII;
	communicationsController.sendPacket[MESSAGE_ID] = (uint8_t)errorType;
	communicationsController.sendPacket[3] = END_PACKET;
	int i = 0;
	while(i < 4){
		TransmitUART(communicationsController.sendPacket[i]);
		i++;
	}
	//communicationsController.errorSent = 1;
}

// Flushes the data register
void USART_Flush( void )
{
	unsigned char data;
	while ( UCSR0A & (1<<RXC0) ) data = UDR0;
}


// Send functions for testing purposes

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