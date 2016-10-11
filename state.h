/*
 *state.h
 * Header for the main controller with the states
 *
 * Created: 5/09/2016 10:47:00 a.m.
 * ELECTENG 311 Smart Fan Project
 * Group 10
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
#endif /* STATE_H_ */