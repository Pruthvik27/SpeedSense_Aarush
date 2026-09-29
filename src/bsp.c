#include "bsp.h"
#include "led.h"

// --- Global Timers ---
volatile uint32_t last_sync_time = 0;
volatile uint32_t system_ms = 0;
volatile uint32_t timer3_10k_ticks = 0;


// === 1. UNIFIED CORE HARDWARE INITIALIZATION ===

void BSP_Init(void) 
{
    // --- ENABLE CORE CLOCKS ---
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO;
    RCC->APB1PCENR |= RCC_APB1Periph_CAN1;

    // Force CAN RX/TX Alternate Function Remap to PB8/PB9
    AFIO->PCFR1 &= ~(3 << 13); 
    AFIO->PCFR1 |=  (2 << 13); 

    // --- GPIOB: RGB LEDs ---
    GPIOB->CFGHR &= ~((0xF << (0 * 4)) | (0xF << (1 * 4)) | (0xF << (4 * 4)) | (0xF << (5 * 4)) | (0xF << (6 * 4)));
    GPIOB->CFGHR |=  ((0x4 << (0 * 4)) | (0xB << (1 * 4)) | (0x1 << (4 * 4)) | (0x1 << (5 * 4)) | (0x1 << (6 * 4)));

    // --- DEFAULT PIN STATES ---
    GPIOB->BSHR = (1 << 12) | (1 << 13) | (1 << 14); // LEDs OFF

    // --- CAN1 SETUP ---
    CAN1->CTLR &= ~(uint32_t)CAN_CTLR_SLEEP;
    CAN1->CTLR |= CAN_CTLR_INRQ;
    CAN1->CTLR &= ~(uint32_t)CAN_CTLR_NART;
    while (!(CAN1->STATR & CAN_STATR_INAK));
    
    CAN1->BTIMR = BAUD_500kbps;
    CAN1->FCTLR |= FCTLR_FINIT; 
    CAN1->sFilterRegister[0].FR1 = (SYNC_PULSE << 21);
    CAN1->sFilterRegister[0].FR2 = 0xFFFFFFFF; 
    CAN1->FAFIFOR = (FIFO << 0); 
    CAN1->FMCFGR  = (0 << 0); 
    CAN1->FSCFGR  = (1 << 0); 
    CAN1->FWR     = (1 << 0); 
    CAN1->FCTLR &= ~FCTLR_FINIT; 

    CAN1->INTENR |= CAN_INTENR_FMPIE0;      
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);   

    CAN1->CTLR &= ~(uint32_t)CAN_CTLR_INRQ;
    while (CAN1->STATR & CAN_STATR_INAK);
}

// === 2. BACKGROUND TIMERS & LEDS ===
// State Tracker for the LEDs
typedef enum {
    LED_STATE_UNKNOWN,
    LED_STATE_HEALTHY,
    LED_STATE_FAULT
} CAN_LED_State_t;

volatile CAN_LED_State_t current_led_state = LED_STATE_UNKNOWN;

void Background_LED_Init(void) {
    // 1. Initialize the PB13/PB14 pins using their init functions
    TIM1_green_Init(); 
    TIM1_red_Init();   

    // 2. Setup TIM3 as the 1ms System Heartbeat
    RCC->APB1PCENR |= RCC_APB1Periph_TIM3;
    TIM3->PSC = 72 - 1;         // 1MHz clock
    TIM3->ATRLR = 1000 - 1;     // 1kHz interrupt trigger (1ms)
    TIM3->DMAINTENR |= TIM_UIE; 
    NVIC_EnableIRQ(TIM3_IRQn);
    TIM3->CTLR1 |= TIM_CEN;
}

void TIM3_IRQHandler(void) __attribute__((interrupt));
void TIM3_IRQHandler(void) {
    if(TIM3->INTFR & TIM_UIF) {
        TIM3->INTFR = ~TIM_UIF; 
        
        // 1. Maintain System Time for main.c
        system_ms++; 

        // 2. Check CAN Health
        bool is_healthy = (system_ms - last_sync_time < 50);

        // 3. AUTOMATED STATE MACHINE
        // Only call the functions if the state has actually CHANGED
        if (is_healthy && current_led_state != LED_STATE_HEALTHY) 
        {
            current_led_state = LED_STATE_HEALTHY;
            TIM1_green_Breathe();
        } 
        else if (!is_healthy && current_led_state != LED_STATE_FAULT) 
        {
            current_led_state = LED_STATE_FAULT;
            TIM1_red_Breathe();
        }
    }
}






void USB_LP_CAN1_RX0_IRQHandler(void)
    __attribute__((interrupt));







// =============================================================================
// 3. CAN TRANSMISSION CONTROLLERS
// ==============================================================================
void CAN_Receive(CAN_Frame_t *frame, uint8_t fifo) {
    if (CAN_RXMI0R_IDE & CAN1->sFIFOMailBox[fifo].RXMIR)
        frame->id = (CAN_RXMI0R_EXID & CAN1->sFIFOMailBox[fifo].RXMIR) >> 3;
    else
        frame->id = (CAN_RXMI0R_STID & CAN1->sFIFOMailBox[fifo].RXMIR) >> 21;

    frame->dlc = CAN_RXMDT0R_DLC & CAN1->sFIFOMailBox[fifo].RXMDTR;
    frame->data.words[0] = CAN1->sFIFOMailBox[fifo].RXMDLR;
    frame->data.words[1] = CAN1->sFIFOMailBox[fifo].RXMDHR;

    if (fifo == 0) CAN1->RFIFO0 |= CAN_RFIFO0_RFOM0;
    else           CAN1->RFIFO1 |= CAN_RFIFO1_RFOM1;
}

int CAN_Transmit(const CAN_Frame_t *frame) {
    int mailbox = -1;

    if (CAN1->TSTATR & CAN_TSTATR_TME0)      mailbox = 0;
    else if (CAN1->TSTATR & CAN_TSTATR_TME1) mailbox = 1;
    else if (CAN1->TSTATR & CAN_TSTATR_TME2) mailbox = 2;

    if (mailbox != -1) {
        CAN1->sTxMailBox[mailbox].TXMIR = (frame->id << 21) & CAN_TXMI0R_STID;
        CAN1->sTxMailBox[mailbox].TXMDTR = frame->dlc & 0x0F;
        
        CAN1->sTxMailBox[mailbox].TXMDLR = frame->data.words[0];
        CAN1->sTxMailBox[mailbox].TXMDHR = frame->data.words[1];
        CAN1->sTxMailBox[mailbox].TXMIR |= CAN_TXMI0R_TXRQ;
    }
    return mailbox;
}