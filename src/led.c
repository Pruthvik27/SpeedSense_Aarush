#include "led.h"
#include "ch32fun.h"

// ============================================================================
// PHYSICAL RGB LED PINS
//
// PA8  = RED
// PA9  = GREEN
// PA10 = BLUE
//
// ACTIVE LOW:
//   LOW  = LED ON
//   HIGH = LED OFF
// ============================================================================

#define LED_RED     (1U << 8)
#define LED_GREEN   (1U << 9)
#define LED_BLUE    (1U << 10)


// ============================================================================
// LED STATE
// ============================================================================

typedef enum
{
    LED_OFF = 0,
    LED_RED_STATE,
    LED_GREEN_STATE,
    LED_BLUE_STATE
} LED_State_t;

static volatile LED_State_t led_state = LED_OFF;

static volatile uint8_t pwm_counter = 0;
static volatile uint8_t brightness = 0;
static volatile int8_t direction = 1;

static volatile uint8_t breathe_tick = 0;


// ============================================================================
// GPIO INITIALIZATION
// ============================================================================

void TIM1_red_Init(void)
{
    // GPIOA clock
    RCC->APB2PCENR |= RCC_IOPAEN;

    // PA8 = RED
    // Normal push-pull output, 50 MHz
    GPIOA->CFGHR &= ~(0xF << 0);
    GPIOA->CFGHR |=  (0x3 << 0);

    GPIOA->BSHR = LED_RED;       // OFF
}


void TIM1_green_Init(void)
{
    // GPIOA clock
    RCC->APB2PCENR |= RCC_IOPAEN;

    // PA9 = GREEN
    // Normal push-pull output, 50 MHz
    GPIOA->CFGHR &= ~(0xF << 4);
    GPIOA->CFGHR |=  (0x3 << 4);

    GPIOA->BSHR = LED_GREEN;     // OFF
}


// ============================================================================
// BLUE INITIALIZATION
// PA10 = BLUE
// ============================================================================

static void LED_Blue_Init(void)
{
    RCC->APB2PCENR |= RCC_IOPAEN;

    // PA10 = BLUE
    // Normal push-pull output, 50 MHz
    GPIOA->CFGHR &= ~(0xF << 8);
    GPIOA->CFGHR |=  (0x3 << 8);

    GPIOA->BSHR = LED_BLUE;      // OFF
}


// ============================================================================
// ALL LEDS OFF
// ============================================================================

static void LED_AllOff(void)
{
    GPIOA->BSHR =
        LED_RED |
        LED_GREEN |
        LED_BLUE;
}


// ============================================================================
// INITIALIZE RGB SYSTEM
// ============================================================================


// ============================================================================
// SET LED STATE
// ============================================================================

void LED_SetState(LED_State_t new_state)
{
    if(led_state != new_state)
    {
        led_state = new_state;

        brightness = 0;
        direction = 1;
        breathe_tick = 0;
    }
}


// ============================================================================
// CAN STATUS FUNCTIONS
// ============================================================================

void TIM1_red_Breathe(void)
{
    LED_SetState(LED_RED_STATE);
}


void TIM1_green_Breathe(void)
{
    LED_SetState(LED_GREEN_STATE);
}


// ============================================================================
// OPTIONAL BLUE
// ============================================================================

void LED_BlueBreathe(void)
{
    LED_SetState(LED_BLUE_STATE);
}


// ============================================================================
// PWM + BREATHING
//
// Called every 1 ms from TIM3.
// ============================================================================

void LED_Tick_1ms(void)
{
    // ------------------------------------------------------------
    // SOFTWARE PWM
    //
    // 100-step PWM.
    // One PWM cycle = 100 ms -> 10 Hz.
    //
    // Active-low:
    // counter < brightness -> ON
    // otherwise            -> OFF
    // ------------------------------------------------------------

    pwm_counter++;

    if(pwm_counter >= 100)
    {
        pwm_counter = 0;
    }

    // ------------------------------------------------------------
    // Select current LED
    // ------------------------------------------------------------

    switch(led_state)
    {
        case LED_RED_STATE:

            GPIOA->BSHR = LED_GREEN | LED_BLUE;

            if(pwm_counter < brightness)
                GPIOA->BCR = LED_RED;
            else
                GPIOA->BSHR = LED_RED;

            break;


        case LED_GREEN_STATE:

            GPIOA->BSHR = LED_RED | LED_BLUE;

            if(pwm_counter < brightness)
                GPIOA->BCR = LED_GREEN;
            else
                GPIOA->BSHR = LED_GREEN;

            break;


        case LED_BLUE_STATE:

            GPIOA->BSHR = LED_RED | LED_GREEN;

            if(pwm_counter < brightness)
                GPIOA->BCR = LED_BLUE;
            else
                GPIOA->BSHR = LED_BLUE;

            break;


        case LED_OFF:

        default:

            LED_AllOff();

            break;
    }

    // ------------------------------------------------------------
    // BREATHE UPDATE
    //
    // Change brightness every 10 ms.
    // ------------------------------------------------------------

    breathe_tick++;

    if(breathe_tick >= 10)
    {
        breathe_tick = 0;

        if(direction)
        {
            if(brightness < 99)
            {
                brightness++;
            }
            else
            {
                direction = 0;
            }
        }
        else
        {
            if(brightness > 0)
            {
                brightness--;
            }
            else
            {
                direction = 1;
            }
        }
    }
}


// ============================================================================
// OLD API COMPATIBILITY
// ============================================================================

void TIM1_red_Blink(void)
{
    LED_SetState(LED_RED_STATE);
}


void TIM1_green_Blink(void)
{
    LED_SetState(LED_GREEN_STATE);
}


// ============================================================================
// OLD TIM1 ISR
//
// TIM1 is no longer used for the RGB indicator.
// ============================================================================

void TIM1_CC_IRQHandler(void) __attribute__((interrupt));

void TIM1_CC_IRQHandler(void)
{
    // Nothing.
}