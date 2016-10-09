/*
 * calibrate.c
 *
 * Created: 1/10/2016 12:19:00 p.m.
 *  Author: emel269
 */ 
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <stdio.h>
 #define F_CPU 8000000UL
  #define MAX_SPEED_VALUE 240 
  #include "prototypes.h"

  extern struct SpeedController speedControl;
  extern struct PwmController pwmController;
  
 ISR(TIMER1_OVF_vect){

	 //Disable interrupt enable on Hall effect
	 ACSR0A &= ~(1<<ACIE0);

	 //Send error status
	 errorStatus = LOCKED;
	 
	 //Disable PWM Channel on TOCC3
	 TOCPMCOE &= ~(1<<TOCC3OE);
	 //Disable PWM Channel on TOCC5
	 TOCPMCOE &= ~(1<<TOCC5OE);
	 // Stop timer 1
	 TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);

 }

void intialiseBlockedDuct(){
	
	unsigned int i;
	unsigned int newRequestedSpeed;
	disableReceiver();
	
	//for(i = 0; i < MAX_SPEED_VALUE ; i++){
		//newRequestedSpeed = (i+30)*10;
		//setRequestedSpeed(newRequestedSpeed);
		////sendSpeedRpm(newRequestedSpeed);
		//while((newRequestedSpeed - speedControl.currentSpeed) >= 20){
			//setRequestedSpeed(newRequestedSpeed);
		//}
//
		//blockedControl.dutyCycleSamples[i] =  (uint8_t)(pwm.dutyCycle * 100.0);
	//}
	
	speedControl.isCalibrated = 1;
	enableReceiver();
}

uint8_t checkBlockDuct(float speed){

	float expectedDutyCycle = 0.0256*(speedControl.currentSpeed) + 7.8292;
		
	if(speedControl.currentSpeed < 900){
	
	return ((pwmController.dutyCycle*100.0) > (1.01*expectedDutyCycle));
	
	}else{

		return ((pwmController.dutyCycle*100.0) > (1.1*expectedDutyCycle));
	}
}

 void intialiseLockedRotor(void){
	 
	 float cutoffRMP = 100;

	 speedControl.lockedRotorCount =  65535-(uint16_t)(F_CPU/((cutoffRMP*3.0/60.0) * speedControl.prescaler));
	 
	 speedControl.lockedRotorDection = 1;

	 //Set Counter1 Count
	 TCNT1 =  speedControl.lockedRotorCount;
	 // Enable overflow interrupts
	 TIMSK1 |= (1<<TOIE1);
 }
