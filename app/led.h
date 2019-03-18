#ifndef __LED_H__
#define __LED_H__
#include "stm8s.h"

#define LED_PORT    GPIOE
#define LED_PIN     GPIO_PIN_5

#define LED_TOGGLE()  GPIO_WriteReverse(LED_PORT, LED_PIN)

void LED_Init(void);
#endif
