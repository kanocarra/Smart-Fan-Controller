#pragma once
#ifndef state
#define state


typedef void(*functionPointer)();
typedef functionPointer(*State)();

State idle();
State receiveData();
State start();
State changeDirection();
State adjustSpeed();
State controlSpeed();
State fanLocked();
State blockedDuct();
State sendStatus();

#endif

