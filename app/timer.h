#ifndef __TIMER_H
#define __TIMER_H

#include "stm8s.h"

void TimerDelay_Init(void);
void TimerDelay(uint32_t delay_10ms);
void TimerBlink_Init(void);
void BlinkNormal(void);
void BlinkFast(void);
void TIM1_SystemTickISR (void) __interrupt(11);
void TIM2_Update_ISR (void) __interrupt(13);

#endif
