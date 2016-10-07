/*
 * state.h
 *
 * Created: 29/08/2016 5:35:09 p.m.
 *  Author: thar729
 */ 

#ifndef STATE_H_
#define STATE_H_

typedef void(*functionPointer)();
typedef functionPointer(*State)();

State idle();
State receiveData();
State start();
State changeDirection();
State controlSpeed();
State fanLocked();
State blockedDuct();
State sleep();
State sendStatus();
State calculatePower();

#endif /* STATE_H_ */