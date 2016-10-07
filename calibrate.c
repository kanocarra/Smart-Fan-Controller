/*
 * calibrate.c
 *
 * Created: 1/10/2016 12:19:00 p.m.
 *  Author: emel269
 */ 
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <stdio.h>

  #define MAX_SPEED_VALUE 240 
  #include "prototypes.h"

  extern struct speedParameters speedControl;
  //extern struct powerParameters power;
  extern struct pwmParameters pwm;
  struct blockedParameters blockedControl;
  
  
//
//void intialiseBlockedDuct(){
//
	//unsigned int i;
	//disableUART();
	//for(i = 0; i < MAX_SPEED_VALUE ; i++){
		//float newRequestedSpeed = (float)(i+30) * 10.0;
		//setRequestedSpeed(newRequestedSpeed);
		//while((newRequestedSpeed - speedControl.currentSpeed) > 10);
		//blockedControl.dutyCycleSamples[i] =  (uint8_t)(pwm.dutyCycle * 100.0);
	//}
	//enableUART();
//}
//
//uint8_t checkBlockDuct(float speed){
//
//
	////blockedControl.dutyCycleSamples[0]=0;
	////blockedControl.dutyCycleSamples[1]=32;
	////blockedControl.dutyCycleSamples[2]=59;
//
	//uint8_t speedIndex = (uint8_t)(speed/100.0);
	//uint8_t expectedDutyCycle = blockedControl.dutyCycleSamples[speedIndex];
//
//
	//return ((pwm.dutyCycle*100) < (0.8*expectedDutyCycle) || (pwm.dutyCycle*100) > (1.2*expectedDutyCycle) );
//
	//
//}