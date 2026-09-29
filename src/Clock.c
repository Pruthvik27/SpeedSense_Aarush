#include <stdint.h>

#define RCC_CFGR0 (*(volatile uint32_t *)0x40021004)
#define RCC_CTLR  (*(volatile uint32_t *)0x40021000)

void Clock_init()
{

    RCC_CTLR |= (1 << 16); // HSE on register set (external crystal is on now)
    while(!(RCC_CTLR & (1<<17))); // wait till rdy bit is 1 and crystal has stabilised
    RCC_CFGR0 |= (1 <<16); // selecting clk source as external crystal
    RCC_CFGR0 &= ~(0b1111 <<18); //PLL multiply to 144 (clear bits)
    RCC_CFGR0 |=  (0b0111 <<18); //PLL multiply to 144 (set bits)
    RCC_CFGR0 &= ~(0b1111 << 4);   // AHB = /1
    RCC_CFGR0 &= ~(0b111 << 8);   // APB1 = /1
    RCC_CFGR0 &= ~(0b111 << 11);  // APB2 = /1

    RCC_CTLR |= (1 << 24); // turn PLL on
    while(!(RCC_CTLR & (1 << 25))); // rdy wait until pll locks
    RCC_CFGR0 &= ~(0b11 << 0); // tell system clk to use PLL clock (first clear bits)
    RCC_CFGR0 |= (0b10 << 0);// now enable bits 10 - use pll
    while(((RCC_CFGR0 >> 2) & 0b11) != 0b10); // wait until cpu actually switches to pll

}
