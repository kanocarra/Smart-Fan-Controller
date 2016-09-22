/*
 * communications.c
 *
 * Created: 5/09/2016 10:32:14 a.m.
 *  Author: emel269
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #define BAUD 9600UL
 #define F_CPU 8000000UL
 #define FAN_ID 2
 #include "prototypes.h"

 struct communicationsPacket packet;

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
	
	PORTA |= (1<< PORTA0); 
	
	unsigned int rX_data = UDR0;
	
	packet.characters[packet.index] = rX_data; 

	switch (packet.index) {
		case SOURCE_ID:
			packet.sourceId = rX_data;
			packet.index++;
			break;

		case DEST_ID:
			packet.destinationId = rX_data;
			
			// Checks that the message is addressed to the smart fan
			if (packet.destinationId == FAN_ID){
				packet.index++;
			} else {
				packet.index = LF;
				reti();
			}
			break;

		case MESSAGE_ID:
			packet.messageId = rX_data;
			packet.index++;
			break;

		case DATA0:
			if(packet.messageId == 83){
				packet.speedValues[packet.speedIndex] = rX_data;
				packet.speedIndex++;
				packet.index++;
				
			} else if(packet.messageId == 63){
				packet.index = LF;
			}
			break;
		
		case DATA1:
			packet.speedValues[packet.speedIndex] = rX_data;
			packet.speedIndex++;
			packet.index++;
			break;
		
		case DATA2:
			packet.speedValues[packet.speedIndex] = rX_data;
			packet.speedIndex++;
			packet.index++;
			break;
		
		case LF:
			if(rX_data == 10) {
				if(packet.messageId == 83) {
					unsigned int speed = packet.speedValues[0] * 1000 + packet.speedValues[1] * 100 +  packet.speedValues[2] * 10;
					setRequestedSpeed(speed);
					packet.speedValues[0] = 0;
					packet.speedValues[1] = 0;
					packet.speedValues[2] = 0;
					packet.transmissionComplete = 1;
				} else if (packet.messageId == 63) {
					sendStatusReport();
					packet.transmissionComplete = 1;
				}
			}
			packet.index = 0;
			packet.speedIndex = 0;
			packet.messageId = 0;
			packet.destinationId = 0;			
			break;
		
		default:
			packet.index = 0;
			packet.speedIndex = 0;
			packet.messageId = 0;
			packet.destinationId = 0;
	}
}

void initialiseUART()
{
	DDRA |= (1<< PORTA0);
	PORTA &= ~(1<< PORTA0); 
	// Set the UBRR value based on the baud rate and clock frequency 
	unsigned int ubrrValue = ((F_CPU)/(BAUD*16)) - 1;

	packet.index = 0;
	packet.messageId = 0;
	packet.speedIndex = 0;

	// Setting the UBRR0 value using its High and Low registers
	UBRR0H = (ubrrValue>>8);
	UBRR0L = ubrrValue;
	
	// Enabling the USART receiver and transmitter and enable receive interrupt
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0); //| (1 << RXCIE0);

	// Set frame size to 8-bits
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);

	// Map Tx to PA7 and Rx to PB2
	REMAP |= (1<<U0MAP);
}

void TransmitUART(uint8_t TX_data)
{
	// Check that the USART Data Register is empty, AND if UCSR0A
	// is all 0s
	while(!(UCSR0A & (1<<UDRE0)));
	
	// Since UDR is empty put the data we want to send into it,
	// then wait for a second and send the following data
	UDR0 = TX_data;
	
}

void sendSpeedRpm(float averageSpeed){
	uint8_t tx_data = (uint8_t)(averageSpeed/10.0);
	TransmitUART(tx_data);
}

void sendCurrent(float RMScurrent){
	uint8_t tx_data = (uint8_t)(RMScurrent * 1000.0);
	TransmitUART(tx_data);
}

void sendStatusReport(void) {

}

