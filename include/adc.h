#ifndef ADC_H
#define ADC_H

#include <stdint.h>

void ADC_Init(void);

void ADC_Start(void);
void ADC_Stop(void);

void ADC_AddChannel(uint8_t channel);
void ADC_ClearChannels(void);
uint16_t ADC_GetValue(uint8_t channel);
void ADC_SetSample(uint8_t channel, uint8_t sampleTime);

#endif