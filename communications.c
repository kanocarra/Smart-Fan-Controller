/*
 * communications.c
 *
 * Created: 5/09/2016 10:32:14 a.m.
 *  Author: emel269
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdio.h>
 #include <avr/sleep.h>
 #define BAUD 9600UL
 #define F_CPU 8000000UL
 #define FAN_ID 2
 #define SW_VERSION 1
 #define END_PACKET 10
  #define R 82
 #define SPEED_REQUEST 83
 #define STATUS_REQUEST 63

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
  
ISR(WDT_vect){
	sleep_disable();
	turnOffWatchDogTimer();
}

ISR(USART0_RX_vect){
	
	unsigned int rX_data = UDR0;
	switch (packet.index) {
		case SOURCE_ID:			
			packet.sourceId = rX_data;
			packet.index++;
			break;

		case DEST_ID:
			packet.destinationId = rX_data;
			
			
			// Checks that the message is addressed to the smart fan otherwise ignores the packet
			if (packet.destinationId == FAN_ID){
				packet.index++;
			} else {
				packet.index = LF;
			}
			break;

		case MESSAGE_ID:
			// Stores the message ID
			packet.messageId = rX_data;
						// If status is requested	
			if(packet.messageId == STATUS_REQUEST){
					packet.transmissionComplete = 1;
					packet.index = LF;
			} else {
				packet.index++;
			}
			break;

		case DATA0:
			// If a new speed is requested
			if(packet.messageId == SPEED_REQUEST){
				packet.speedValues[packet.speedIndex] = rX_data;
				packet.speedIndex++;
				packet.index++;
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
			if(rX_data == END_PACKET) {
				if(packet.messageId == 83) {
					packet.requestedSpeed = packet.speedValues[0] * 1000 + packet.speedValues[1] * 100 +  packet.speedValues[2] * 10;
					packet.speedValues[0] = 0;
					packet.speedValues[1] = 0;
					packet.speedValues[2] = 0;
					packet.transmissionComplete = 1;
				} 
			}
			packet.index = 0;
			packet.speedIndex = 0;
			packet.destinationId = 0;			
			break;
		
		default:
			packet.index = 0;
			packet.speedIndex = 0;
			packet.destinationId = 0;
	}
}

ISR(USART0_START_vect){
	
	// Disable sleep mode
	sleep_disable();
	
	packet.transmissionStart = 1;
	
	// Disable receive start interrupt
	UCSR0D &= ~(1<<SFDE0) & ~(1<<RXSIE0);
	
	
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
	enableUART();
	
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
	// Check that the USART Data Register is empty, AND if UCSR0A
	// is all 0s
	while(!(UCSR0A & (1<<UDRE0)));
	
	// Since UDR is empty put the data we want to send into it,
	// then wait for a second and send the following data
	UDR0 = TX_data;
	
}

void sendStatusReport(unsigned int requestedSpeed, float currentSpeed, float power, unsigned int error) {
	packet.sendPacket[SOURCE_ID] = FAN_ID;
	packet.sendPacket[DEST_ID] = packet.sourceId;
	packet.sendPacket[MESSAGE_ID] = R;
	packet.sendPacket[DATA0] = SW_VERSION;
	packet.sendPacketIndex = DATA1;
	
	convertToPacket(requestedSpeed);
	convertToPacket((unsigned int)currentSpeed);
	
	packet.sendPacket[packet.sendPacketIndex] = (uint8_t)(power * 10.0);
	packet.sendPacketIndex++;
	
	if(error == NONE){
		packet.sendPacket[packet.sendPacketIndex] = '-';
	} else if(error == LOCKED){
		packet.sendPacket[packet.sendPacketIndex] = 'L';
	} else if (error == BLOCKED){
		packet.sendPacket[packet.sendPacketIndex] = 'B';
	}
	
	packet.sendPacketIndex++;
	
	packet.sendPacket[packet.sendPacketIndex] = END_PACKET;
	int i = 0;
	while(i <= packet.sendPacketIndex){
		TransmitUART(packet.sendPacket[i]);
		i++;
	}
	packet.sendPacketIndex = 0;
	
}

void disableUART(void){
	// Disable UART receive interrupt
	UCSR0B &= ~(1<RXCIE0) & ~(1<<RXEN0);
}

void enableUART(void) {
	// Enable UART receive interrupt
	packet.transmissionComplete = 0;
	UCSR0B |= (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
}

void convertToPacket(unsigned int speed){
		unsigned int factor = 10000;
		unsigned int convertNumber = speed;

		//if(num % 1 != 0):
		//decimal = num % 1
		//print(decimal)

		//num = int(num)
		while(factor>10){
			factor = factor/10;
			packet.sendPacket[packet.sendPacketIndex] = convertNumber/factor;
			convertNumber = convertNumber % factor;
			packet.sendPacketIndex++;
		}
}

void sendError(char errorType){
	packet.sendPacket[SOURCE_ID] = FAN_ID;
	packet.sendPacket[DEST_ID] = packet.sourceId;
	packet.sendPacket[MESSAGE_ID] = 76;
	packet.sendPacket[3] = END_PACKET;
	int i = 0;
	while(i < 4){
		TransmitUART(packet.sendPacket[i]);
		i++;
	}
}

void initialiseWatchDogTimer(void){
	
	// Write config change protection with watch dog signature
	CCP = 0xD8;
	
	// Enable watchdog timer interrupt and set delay of 1s
	WDTCSR |= (1<< WDIE) | (1<<WDP2) | (1<<WDP1);
	
	// CLear watchdog reset flag
	MCUSR &= ~(1<< WDRF); 

}

void turnOffWatchDogTimer(void){
	
	// Write config change protection with watch dog signature
	CCP = 0xD8;
	
	// Clear watchdog
	WDTCSR &= ~(WDTCSR);
}

void sendSpeedRpm(float averageSpeed){
	uint8_t tx_data = (uint8_t)(averageSpeed/10.0);
	TransmitUART(tx_data);
}

void sendCurrent(float RMScurrent){
	uint8_t tx_data = (uint8_t)(RMScurrent * 10000.0);
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
