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
 #include "prototypes.h"

 struct communicationsPacket packet;

ISR(USART0_RX_vect){
	
	unsigned int rX_data = UDR0;
	
	packet.characters[packet.index] = rX_data; 

	if(packet.speedIndex == 3)
	{
		packet.index = 0;
		packet.messageId = 0;
		packet.speedIndex = 0;
		unsigned int speed = packet.speedValues[0] * 1000 + packet.speedValues[1] * 100 +  packet.speedValues[2] * 10;
		setRequestedSpeed(speed);
	}

	if(packet.index >= 3){
		packet.speedValues[packet.index - 3] = rX_data;
		packet.speedIndex++;
	}

	if(packet.index == 2){
		packet.messageId = rX_data;	
	}

	packet.index++;
}

void initialiseUART()
{
	
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

