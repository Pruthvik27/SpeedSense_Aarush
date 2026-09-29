// #ifndef MT6835_H
// #define MT6835_H

// #include "ch32fun.h"
// #include <stdint.h>

// #define CS_LOW()   GPIOA->BSHR = (1 << (4 + 16))
// #define CS_HIGH()  GPIOA->BSHR = (1 << 4)

// uint32_t MT6835_ReadAngleRaw(void);
// // uint8_t SPI1_Transfer(uint8_t data);

// #endif  

#ifndef MT6835_H
#define MT6835_H

#include "ch32fun.h"
#include <stdint.h>
#include <stdbool.h>

// Initiates a non-blocking SPI sequence to read MT6835 angle
bool MT6835_TriggerRead(void);

// Processes the raw SPI frame into a 21-bit integer
bool MT6835_GetRawAngleIfReady(uint32_t *raw_angle);
void print_angle(float angle);
void print_int(int32_t num);
#endif