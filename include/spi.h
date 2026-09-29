// #ifndef SPI_H
// #define SPI_H 

// #include "ch32fun.h"
// #include <stdint.h>

// #define CS_LOW()   GPIOA->BSHR = (1 << (4 + 16))
// #define CS_HIGH()  GPIOA->BSHR = (1 << 4)

// void SPI1_init(void);
// uint8_t SPI1_Transfer(uint8_t data);
// // uint32_t MT6835_ReadAngleRaw(void);

// #endif

#ifndef SPI_H
#define SPI_H 

#include "ch32fun.h"
#include <stdint.h>
#include <stdbool.h>

#define CS_LOW()   GPIOA->BSHR = (1 << (4 + 16))
#define CS_HIGH()  GPIOA->BSHR = (1 << 4)

void SPI1_init(void);

// Non-blocking SPI transfer function
bool SPI1_Transfer_IT(const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length);

// Query if SPI driver is busy transmitting
bool SPI1_IsBusy(void);

#endif