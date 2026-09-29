// #include "MT6835.h"
// #include "spi.h"

// uint32_t MT6835_ReadAngleRaw(void)
// {
//     uint8_t a, b, c ;

//     CS_LOW();
    

//     SPI1_Transfer(0xA0); 
//     SPI1_Transfer(0x03);

//     a = SPI1_Transfer(0x00);
//     b = SPI1_Transfer(0x00); 
//     c = SPI1_Transfer(0x00);

//     CS_HIGH();

//     uint32_t angle = ((uint32_t)a << 13) | ((uint32_t)b << 5)  | ((uint32_t)c >> 3);

//     return angle;
// }


#include "MT6835.h"
#include "spi.h"
#include <stddef.h>

static uint8_t tx_frame[5] = {0xA0, 0x03, 0x00, 0x00, 0x00};
static uint8_t rx_frame[5] = {0};

bool MT6835_TriggerRead(void)
{
    return SPI1_Transfer_IT(tx_frame, rx_frame, 5);
}

bool MT6835_GetRawAngleIfReady(uint32_t *raw_angle)
{
    if (SPI1_IsBusy()) {
        return false; // Transfer still ongoing in background
    }

    // Extract 21-bit angle value from RX bytes (A, B, C)
    uint8_t a = rx_frame[2];
    uint8_t b = rx_frame[3];
    uint8_t c = rx_frame[4];
   
    *raw_angle = ((uint32_t)a << 13) | ((uint32_t)b << 5) | ((uint32_t)c >> 3);

    return true;
}
void print_angle(float angle)
{
    int32_t integral = (int32_t)angle;
    int32_t fractional = (int32_t)((angle - integral) * 100.0f);
    
    if (fractional < 0) fractional = -fractional;
    
    print_string("Angle: ");
    print_int(integral);
    print_string(".");
    if (fractional < 10) print_string("0");
    print_int(fractional);
    print_string(" deg\r\n");
}

void print_int(int32_t num)
{
    char buf[12];
    int i = 0;
    
    if (num == 0) { print_string("0"); return; }
    if (num < 0)  { print_string("-"); num = -num; }
    
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    
    while (i > 0) {
        while(!(USART2->STATR & USART_STATR_TXE));
        USART2->DATAR = buf[--i];
    }
}
