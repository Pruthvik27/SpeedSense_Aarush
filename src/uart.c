#include <stdio.h>
#include "ch32fun.h"

void Uart_Init(void)
{
    RCC->APB2PCENR |= (1 << 2);      // GPIOA
    RCC->APB1PCENR |= (1 << 17);     // USART2

    GPIOA->CFGLR &= ~(0b1111 << 8);
    GPIOA->CFGLR |=  (0b1001 << 8);  // PA2 AF Push Pull

    GPIOA->CFGLR &= ~(0b1111 << 12);
    GPIOA->CFGLR |=  (0b0100 << 12); // PA3 Floating Input

    USART2->CTLR1 &= ~(1 << 13);     // Disable USART

    USART2->CTLR1 &= ~(1 << 12);     // 8 bit

    USART2->CTLR2 &= ~(0b11 << 12);  // 1 stop bit

    USART2->CTLR2 &= ~(1 << 11);     // Async mode

    USART2->BRR = 0;

    USART2->BRR |= (0b111010100 << 4);     // Mantissa = 468
    USART2->BRR |= (0b1100);               // Fraction = 12

    USART2->CTLR1 |= (1 << 13);      // USART Enable
    USART2->CTLR1 |= (1 << 3);       // TX Enable
    USART2->CTLR1 |= (1 << 2);       // RX Enable
}

void print_string(const char *str)
{
    while(*str)
    {
        while(!(USART2->STATR & (1 << 7)));

        USART2->DATAR = *str++;
    }
}