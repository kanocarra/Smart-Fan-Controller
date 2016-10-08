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
	
	return ((pwm.dutyCycle*100.0) > (1.01*expectedDutyCycle));
	
	}else{

		return ((pwm.dutyCycle*100.0) > (1.1*expectedDutyCycle));
	}
}