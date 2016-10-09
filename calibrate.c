/*
 * calibrate.c
 *
 * Created: 1/10/2016 12:19:00 p.m.
 *  Author: emel269
 */ 
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <stdio.h>
  #include <math.h>


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

	//polynomial cal
	float a = 2.0 * (pow(10, -12.0));
	float b = -1.0 * (pow(10, -8.0));
	float c = 2.0 * (pow(10, -5.0));
	float d = 0.0218;
	float e = 4.8874;

	//polynomial calculations
	float expectedDutyCycle  = a*(pow(speed, 4)) - b*(pow(speed, 3)) + c*(pow(speed, 2)) + d*(speed) + e;
	//linear calculations 
	float expectedDutyCycleOne = 0.0256*(speed) + 7.8292;
		
	if(speed < 350){
	
	return ((pwm.dutyCycle*100.0) > (1.1*expectedDutyCycle));
	
	}else if(speed>2000){

		return ((pwm.dutyCycle*100.0) > (1.1*expectedDutyCycleOne));
	}
}