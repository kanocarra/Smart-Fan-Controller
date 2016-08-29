/*
 * LED_BLINK.c
 *
 * Created: 15/08/2016 4:11:57 p.m.
 * Author : emel269
 */ 

#include <avr/io.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000

//functrion declarations
void initialiseAnalogComparator(void);
void initialisePWMtimer(void);



ISR(ANA_COMP0_vect)
{
	//disable interrupts
	ACSR0A &= ~(1<<ACIE0);
	//turn LED(0) on & all others off & reset
	PORTB ^= (1<<PORTB0);
	PORTB ^= (1<<PORTB1);
	//toggle between rising and falling
	ACSR0A ^= (1<<ACIS00);
	//enable interrupts once service done
	ACSR0A |= (1<<ACIE0);
}

int main(void)
{
	// PIN B0 B1 is output 2x LED's
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);

	DDRA |= (1<<PORTA2);

	//initialize Analog Comparator
	initialiseAnalogComparator();
	//initialise PWM timer
	initialisePWMtimer();

	//clear port B
	PORTB &= ~(PORTB);

	//turn LED on initially for polling 
	PORTB |= (1<<PORTB0);

	//enable global interrupts
	sei();

	while (1) {	
	}
}

void initialiseAnalogComparator(void){

	// clear control and status register A
	 ACSR0A &= ~(ACSR0A);

	// clear control and status register B
	 ACSR0B &= ~(ACSR0B);

	//Set hysteresis level of 50mV
	ACSR0B |= (1<<HSEL0) | (1<<HLEV0);

	//enable output comparator
	ACSR0B |= (1<<ACOE0);

	//set rising edge
	ACSR0A |= (1<<ACIS01) | (1<<ACIS00);
	
	//initialise interrupt enable
	ACSR0A |= (1<<ACIE0);
}

void initialisePWMtimer(void){

	//initialise variables
	unsigned int prescaler = 1;
	unsigned int PWMfreq = 20000;
	unsigned int count = (F_CPU/(2*prescaler*PWMfreq)) - 1;
	unsigned int timerCount = (2^16) - count;
	double dutyCycle = 0.5;
	unsigned int compareCount;
	
	//clear control register A (Timer 2)
	TCCR2A &= ~(TCCR2A);

	//clear control register B (Timer 2)
	TCCR2B &= ~(TCCR2B);

	//clear timer output compare marker register A1
	TOCPMSA1 &= ~(TOCPMSA1);

	//clear timer output compare marker register A0
	TOCPMSA0 &= ~(TOCPMSA0);

	//Compare Output Mode, Fast PWM (toggle on output compare match)
	TCCR2A |= (1<<COM2A0);

	//10-bit Fast PWM, Waveform Generator Mode
	TCCR2A |= (1<<WGM20) | (1<<WGM21);
	TCCR2B |= (1<<WGM22);

	//set clock speed (16MHz)
	TCCR2B |= (1<<CS20);

	//set output to TOCC1
	TOCPMSA1 |= (1<<TOCC1S1);
	//TOCPMSA0 |= (1<<TOCC3S1);  

	//clearing register
	TOCPMCOE &= ~(TOCPMCOE);

	//set channel 1 as output/enable
	TOCPMCOE |= (1<<TOCC1OE);
	
	//initialising max count(overflow)
	TCNT2H = (timerCount>>8);
	TCNT2L = timerCount;

	compareCount = timerCount + (count*dutyCycle);
	
	//initialise output compare registers
	OCR2AH = (compareCount>>8);
	OCR2AL = compareCount;
	TIMSK2 &= ~(TIMSK2);

	//set output compare A and B interrupt enable
	//TIMSK2 |= (1<<OCIE2B); 
	

}