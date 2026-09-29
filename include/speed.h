#ifndef SPEED_H
#define SPEED_H

#include <stdint.h>

void Speed_Init(void);
uint8_t Speed_getSpeed(void);
uint32_t Speed_getRPM(void);
uint32_t Speed_getFrequency(void);
void Speed_Reset(void);

#endif

