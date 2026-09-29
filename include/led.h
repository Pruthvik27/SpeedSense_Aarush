#ifndef LED_H
#define LED_H

#include "ch32fun.h"
#include <stdint.h>

#define PWM_ATRLR       999
#define PWM_PSC         143


void clock_init(void);

void TIM1_red_Init(void);
void TIM1_green_Init(void);


void TIM1_red_Blink(void);
void TIM1_green_Blink(void);

void TIM1_red_Breathe(void);
void TIM1_green_Breathe(void);

void TIM1_CC_IRQHandler(void);
void delay_ms(uint32_t ms);



#endif

