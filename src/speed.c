/*
Public API
void Speed_Init(void);

uint32_t Speed_getSpeed(void);
uint32_t Speed_getRPM(void);
uint32_t Speed_getFrequency(void);

void Speed_Reset(void);






    */    
        
    #include <stdint.h>
    #include "ch32fun.h"
    #include "speed.h"

    #define RCC_APB1PCENR (*(volatile uint32_t *) 0x4002101C)// enable apb1 clk
    #define RCC_APB2PCENR (*(volatile uint32_t *)0x40021018)// enableapb2 clk
    #define GPIOA_CFGLR (*(volatile uint32_t *)0x40010800)// gpio A enable lower for input

    static volatile uint32_t previous_capture = 0;
    static volatile uint32_t current_capture = 0;
    static volatile uint32_t difference = 0;

    static volatile uint32_t frequency = 0;
    static volatile uint32_t rpm = 0;
    static volatile uint8_t speed = 0;

    static volatile uint32_t overflow_count = 0;
    static volatile uint8_t first_capture = 1;



    #define TIM2_CTLR1  (*(volatile uint32_t *)0x40000000) //on or off timer
    #define TIM2_CNT    (*(volatile uint32_t *)0x40000024) // counter
    #define TIM2_PSC    (*(volatile uint32_t *)0x40000028) // prescaler divider
    #define TIM2_ATRLR  (*(volatile uint32_t *)0x4000002C) // max limit round off

    #define TIM2_INTFR (*(volatile uint32_t *)0x40000010)

    #define GPIOA_OUTDR (*(volatile uint32_t *)0x4001080C)


    #define TIM2_CHCTLR1 (*(volatile uint32_t *)0x40000018) // capture control enable register
    #define TIM2_CCER (*(volatile uint32_t *)0x40000020) // tim2 capture enable register

    #define TIM2_CH1CVR (*(volatile uint32_t *)0x40000034)
    #define TIM2_DMAINTENR (*(volatile uint32_t *)0x4000000C) //interrupt enable register

    #define PFIC_IENR1 (*(volatile uint32_t *)0xE000E100) // cpu <#31 value interrupt enable (tim2 = 28)


    #define TIMER_FREQ 100000   //72/prescale value so5000 ticks every sec






void TIM2_IRQHandler(void) __attribute__((interrupt));
void TIM2_IRQHandler(void)
{
    //overflow check
    if (TIM2_INTFR & (1 << 0))
    {
        overflow_count++;

        // Timeout (~2 overflows)
        if(overflow_count >= 2)
        {
            frequency = 0;
            rpm = 0;
            speed = 0;
        }

        TIM2_INTFR &= ~(1 << 0);
    }

    // capture
    if (TIM2_INTFR & (1 << 1))
    {
        current_capture = TIM2_CH1CVR;

        // Ignore first capture
        if(first_capture)
        {
            previous_capture = current_capture;
            overflow_count = 0;
            first_capture = 0;

            TIM2_INTFR &= ~(1 << 1);
            return;
        }

        uint32_t ticks;

        if(current_capture >= previous_capture)
        {
            ticks =
                overflow_count * 65536+(current_capture - previous_capture);
        }
        else
        {
            ticks =
                (overflow_count - 1) * 65536+ (65536- previous_capture) + current_capture;
        }

        difference = ticks;

        previous_capture = current_capture;
        overflow_count = 0;

        if(difference != 0)
        {
            frequency = TIMER_FREQ / difference;

            // Two black strips
            rpm = (frequency * 60) / 2;

            speed = (frequency * 2298) / 1000;
        }

        TIM2_INTFR &= ~(1 << 1);
    }
}




void Speed_Init(void)
{
    RCC_APB1PCENR |= (1 << 0);     // TIM2 clock enable
    RCC_APB2PCENR |= (1 << 2);     // GPIOA clock enable

    // PA0 = floating input
    GPIOA_CFGLR &= ~(0b1111 << 0);
    GPIOA_CFGLR |=  (0b0100 << 0);

    // Timer configuration
    TIM2_PSC   = 719;
    TIM2_ATRLR = 65535;
    TIM2_CNT   = 0;

    // Channel 1 configured as input capture on TI1 (PA0)
    TIM2_CHCTLR1 &= ~(0b11 << 0);
    TIM2_CHCTLR1 |=  (0b01 << 0);

    // No input filter
    TIM2_CHCTLR1 &= ~(0b1111 << 4);

    // Falling-edge capture
    TIM2_CCER &= ~((1 << 0) | (1 << 1));
    TIM2_CCER |=  (1 << 0);    // CC1E: enable capture
    TIM2_CCER |=  (1 << 1);    // CC1P: falling edge

    // Enable capture and overflow interrupts
    TIM2_DMAINTENR |= (1 << 1);   // CC1 interrupt
    TIM2_DMAINTENR |= (1 << 0);   // Update/overflow interrupt

    NVIC_EnableIRQ(TIM2_IRQn);

    // Start timer
    TIM2_CTLR1 |= (1 << 0);
}

uint8_t Speed_getSpeed(void)
{
    
    return speed; // from iterrupt, same for all
}

uint32_t Speed_getRPM(void)
{
    return rpm;
}

uint32_t Speed_getFrequency(void)
{
    return frequency;
}

void Speed_Reset(void)
{
    previous_capture = 0;
    current_capture = 0;
    difference = 0;
    frequency = 0;
    rpm = 0;
    speed = 0;
    TIM2_CNT = 0;
    overflow_count = 0;
    first_capture = 1;
}






