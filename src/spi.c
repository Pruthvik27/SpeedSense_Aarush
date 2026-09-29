// #include "spi.h"

// void SPI1_init(void)
// {

//     RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1;

//     GPIOA->CFGLR &= ~((0xF << (4*4)) | 
//                       (0xF << (5*4)) | 
//                       (0xF << (6*4)) | 
//                       (0xF << (7*4)));

//     GPIOA->CFGLR |=  ((0x3 << (4*4)) |  
//                       (0xB << (5*4)) |   
//                       (0x4 << (6*4)) |   
//                       (0xB << (7*4)));   
//     GPIOA->BSHR = (1 << 4);

//     // 4. Configure SPI CTLR1
//     SPI1->CTLR1 = 0;
    
//     SPI1->CTLR1 |= SPI_CTLR1_MSTR |        
//                    SPI_CTLR1_BR_2 |      
//                    SPI_CTLR1_BR_1 |
//                    SPI_CTLR1_CPOL |       
//                    SPI_CTLR1_CPHA |        
//                    SPI_CTLR1_SSM  |       
//                    SPI_CTLR1_SSI;          

//     SPI1->CTLR1 |= SPI_CTLR1_SPE;          
// }

// uint8_t SPI1_Transfer(uint8_t data)
// {
//     while(!(SPI1->STATR & SPI_STATR_TXE));
//     SPI1->DATAR = data;

//     while(!(SPI1->STATR & SPI_STATR_RXNE));
//     return SPI1->DATAR;
// }


#include "spi.h"


typedef struct {
    const uint8_t *tx_buf;
    uint8_t       *rx_buf;
    uint16_t       length;
    uint16_t       tx_idx;
    uint16_t       rx_idx;
    volatile bool  busy;
} SPI_TransferState_t;

static SPI_TransferState_t spi_state = {0};

void SPI1_init(void)
{
    // 1. Enable GPIOA and SPI1 Clocks
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1;

    // 2. Configure PA4(CS), PA5(SCK), PA6(MISO), PA7(MOSI)
    GPIOA->CFGLR &= ~((0xF << (4*4)) | 
                      (0xF << (5*4)) | 
                      (0xF << (6*4)) | 
                      (0xF << (7*4)));

    GPIOA->CFGLR |=  ((0x3 << (4*4)) |  // Output Push-Pull (CS)
                      (0xB << (5*4)) |  // Alternate Push-Pull (SCK)
                      (0x4 << (6*4)) |  // Floating Input (MISO)
                      (0xB << (7*4)));  // Alternate Push-Pull (MOSI)

    // CS_HIGH();

    // 3. Configure SPI CTLR1
    SPI1->CTLR1 = 0;
    SPI1->CTLR1 |= SPI_CTLR1_MSTR |        
                   SPI_CTLR1_BR_2 |      
                   SPI_CTLR1_BR_1 |
                   SPI_CTLR1_CPOL |       
                   SPI_CTLR1_CPHA |        
                   SPI_CTLR1_SSM  |       
                   SPI_CTLR1_SSI;          

    SPI1->CTLR1 |= SPI_CTLR1_SPE; 

    // 4. Enable NVIC interrupt for SPI1 in RISC-V core
    NVIC_EnableIRQ(SPI1_IRQn);
}

bool SPI1_IsBusy(void)
{
    return spi_state.busy;
}

bool SPI1_Transfer_IT(const uint8_t *tx_buf, uint8_t *rx_buf, uint16_t length)
{
    if (spi_state.busy || length == 0) return false;

    spi_state.tx_buf = tx_buf;
    spi_state.rx_buf = rx_buf;
    spi_state.length = length;
    spi_state.tx_idx = 0;
    spi_state.rx_idx = 0;
    spi_state.busy   = true;

    CS_LOW();

    // Enable RX interrupts; TX interrupt triggered when TXE empties
    SPI1->CTLR2 |= SPI_CTLR2_RXNEIE | SPI_CTLR2_TXEIE;

    return true;
}

// Hardware Interrupt Service Routine
void SPI1_IRQHandler(void) __attribute__((interrupt));
void SPI1_IRQHandler(void)
{
    uint16_t statr = SPI1->STATR;

    // --- RECEIVE HANDLER (RXNE) ---
    if ((statr & SPI_STATR_RXNE) && (SPI1->CTLR2 & SPI_CTLR2_RXNEIE)) {
        uint8_t received_byte = SPI1->DATAR;

        if (spi_state.rx_buf != NULL) {
            spi_state.rx_buf[spi_state.rx_idx] = received_byte;
        }
        spi_state.rx_idx++;

        // End of Transfer Condition
        if (spi_state.rx_idx >= spi_state.length) {
            SPI1->CTLR2 &= ~(SPI_CTLR2_RXNEIE | SPI_CTLR2_TXEIE); // Disable SPI interrupts
            CS_HIGH();
            spi_state.busy = false; // Mark complete
        }
    }

    // --- TRANSMIT HANDLER (TXE) ---
    if ((statr & SPI_STATR_TXE) && (SPI1->CTLR2 & SPI_CTLR2_TXEIE)) {
        if (spi_state.tx_idx < spi_state.length) {
            uint8_t tx_byte = (spi_state.tx_buf != NULL) ? spi_state.tx_buf[spi_state.tx_idx] : 0x00;
            SPI1->DATAR = tx_byte;
            spi_state.tx_idx++;
        } else {
            // Disable TX buffer interrupt once full sequence written
            SPI1->CTLR2 &= ~SPI_CTLR2_TXEIE;
        }
    }
}
