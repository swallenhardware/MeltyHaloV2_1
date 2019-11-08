/*
 * time.c
 *
 *  Created on: Apr 16, 2019
 *      Author: swallen
 */

#include "time.h"

TIM_TypeDef * microTimInst = TIM5;
TIM_TypeDef * milliTimInst = TIM4;

void initTimers() {
	//SETUP THE MICROSECONDS TIMER
	__HAL_RCC_TIM4_CLK_ENABLE();
	__HAL_RCC_TIM5_CLK_ENABLE();

	//make sure the timers are disabled
	microTimInst->CR1 = 0;

	//set up the prescaler on the low counter
	microTimInst->PSC = (MAIN_CLOCK/2)-1;//gives 1MHz, or 1us period

	//reset the timer counts
	microTimInst->CNT = 0;

	//SETUP THE MILLISECONDS TIMER
	__HAL_RCC_TIM4_CLK_ENABLE();
	milliTimInst->CR1 = 0;//make sure the timer is disabled

	milliTimInst->PSC = (MAIN_CLOCK*1000/2)-1;//gives 1KHz, or 1ms period
	milliTimInst->CNT = 0;

	//manually generate an update event so the prescaler loads into the shadow registers
	//this took a ridiculous two hours to figure out. Just so you know
	milliTimInst->EGR |= 0x0001;
	microTimInst->EGR |= 0x0001;

	//ENABLE THE TIMERS
	milliTimInst->CR1 = 1;
	microTimInst->CR1 = 1;
	resetTimers();
}

uint32_t getMicroseconds() {
	return microTimInst->CNT;
}

uint32_t getMilliseconds() {
	return milliTimInst->CNT;
}

void resetTimers() {
	microTimInst->CNT = 0;
	milliTimInst->CNT = 0;
}

