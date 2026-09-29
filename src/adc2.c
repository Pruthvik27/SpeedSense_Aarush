// /*
// File: adc.c
// MCU: CH32V203C6T6


// Public API:
// ADC_Init()
// ADC_AddChannel()
// ADC_SetSample()
// ADC_Start()
// ADC_Stop()
// ADC_GetValue()
// */




// #include <stdint.h>
// #include "ch32fun.h"
// #include "uart.h"
// //static variable

// volatile uint16_t adc_val;
// volatile uint8_t conv_cmplt;
// uint8_t channelList[16]; // array for initialised channels
// volatile uint16_t convertedValues[16]; // values stored in this after each cycle, can be accessed when function called
// uint8_t channelCount = 0; // count for number of user added channels
// volatile uint8_t currentConversion = 0; // sees which channel is being converted next to be stored in array


// //register defenitions
// #define ADC_STATR      (*(volatile uint32_t *)0x40012400)//status
// #define ADC_CTLR1      (*(volatile uint32_t *)0x40012404)//control reg 1
// #define ADC_CTLR2      (*(volatile uint32_t *)0x40012408)//cntrl2

// #define ADC_SAMPTR1    (*(volatile uint32_t *)0x4001240C)//Sampling time for 10-17 channel
// #define ADC_SAMPTR2    (*(volatile uint32_t *)0x40012410)//sampling time for 0-9 channel

// #define ADC_RSQR1      (*(volatile uint32_t *)0x4001242C)// sequence reg 1
// #define ADC_RSQR2      (*(volatile uint32_t *)0x40012430)// reg2
// #define ADC_RSQR3      (*(volatile uint32_t *)0x40012434)// reg3

// #define ADC_RDATAR     (*(volatile uint32_t *)0x4001244C)//adc conversion value

// #define RCC_APB2PCENR (*(volatile uint32_t *)0x40021018) // clock enable register apb2
// #define RCC_CFGR0     (*(volatile uint32_t *)0x40021004)

// #define RCC_APB2PRSTR (*(volatile uint32_t *)0x4002100C)








// // bit defenitions

// #define ADC_ADON (1<< 0)// enable adc
// #define ADC_CONT (1<< 1)// continuous conversion loop
// #define ADC_CAL (1<< 2) // calibrate adc
// #define ADC_RSTCAL (1<< 3)// reset adc calibration
// #define ADC_SWSTART (1<< 22)// start conversion (cpu not external)
// #define ADC_SCAN (1<< 8) // keep scanning sq1, sq2,......
// #define ADC_EOCIE (1<< 5)// enable end of conversion interrupts
// #define ADC_EOC (1<< 1) // conversion finished if bit 1 (status flag)
// #define RCC_ADC1EN (1 << 9) // adc clock enable regisster 
// #define ADC_EXTTRIG (1 << 20)// trigger enable






// // public functions
// void ADC_Init(void)
// {

//     RCC_APB2PCENR |= RCC_ADC1EN;  

//     RCC_CFGR0 &= ~(3 << 14);
//     RCC_CFGR0 |=  (3 << 14);  

//     RCC_APB2PRSTR |= RCC_ADC1EN;
//     RCC_APB2PRSTR &= ~RCC_ADC1EN;

//     ADC_CTLR2 |= ADC_ADON;           

//     ADC_CTLR2 |= ADC_RSTCAL;
//     while(ADC_CTLR2 & ADC_RSTCAL);

//     ADC_CTLR2 |= ADC_CAL;
//     while(ADC_CTLR2 & ADC_CAL);

//     ADC_CTLR1 &= ~ADC_SCAN;
//     ADC_CTLR1 |= ADC_SCAN; //enable scanning of multiple channels

//     ADC_CTLR2 &= ~ADC_CONT;
//     // ADC_CTLR2 |= ADC_CONT; // enable continuous scanning


//     ADC_CTLR1 |= ADC_EOCIE;
//     NVIC_EnableIRQ(ADC1_2_IRQn);

    

// }

// uint16_t ADC_GetValue(uint8_t index)
// {
//     return convertedValues[index];
// }

// void ADC_Stop(void)
// {
//     ADC_CTLR2 &= ~ADC_CONT; // stops continuous mode, adc stops after last channel
//     ADC_CTLR2 &= ~ADC_ADON; // shut down adc

// }

// void ADC_Start(void)
// {

//     currentConversion = 0;
//     ADC_STATR = 0;
//     ADC_CTLR2 &= ~(7 << 17);      // Clear EXTSEL
//     ADC_CTLR2 |=  (7 << 17);      // SWSTART trigger

//     ADC_CTLR2 |= ADC_EXTTRIG;
//     ADC_CTLR2 |= ADC_SWSTART;
// }


// void ADC_SetSample(uint8_t channel, uint8_t sampleTime)
// {

//     if(channel <= 9) //tr2 for channel 0-9
//     {
//         ADC_SAMPTR2 &= ~(0b111 << (channel * 3)); // tr is 3 bits
//         ADC_SAMPTR2 |= (sampleTime << (channel * 3));
//     }
//     else if(channel <= 17) // tr1 ffor rest
//     {
//         ADC_SAMPTR1 &= ~(0b111 << ((channel - 10) * 3)); // clear first
//         ADC_SAMPTR1 |= (sampleTime << ((channel - 10) * 3)); // then set *3 because 3 bits
//     }

// }


// void ADC_AddChannel(uint8_t channel)
// {
//     channelList[channelCount++] = channel;// first store channel in array when called
// }

// void ADC_ClearChannels(void)
// {
//     channelCount = 0;
//     currentConversion = 0;
// }


// void ADC_Program(uint8_t* seq_list, uint8_t seq_len)
// {

//     seq_len = seq_len > 16 ? 16 : seq_len;
//     if(seq_len> 12){
//         for(uint8_t i = 12; i < seq_len; i++){
//             ADC_RSQR1 |= (seq_list[i] << ((i - 12) * 5));
//         }
//     }
//     if(seq_len > 6){
//         for(uint8_t i = 6; i < (seq_len > 12 ? 12 : seq_len); i++){
//             ADC_RSQR2 |= (seq_list[i] << ((i - 6) * 5));
//         }
//     }
//     if(seq_len > 0){
//         for(uint8_t i = 0; i < (seq_len > 6 ? 6 : seq_len) ; i++){
//         ADC_RSQR3 |= (seq_list[i] << (i * 5)); 
//     }
//     }

//     ADC_RSQR1 &= ~(0xF << 20); // clear Rlen
//     ADC_RSQR1 |= ((seq_len - 1)<<20); // set Rlen

// }

// void ADC1_2_IRQHandler(void) __attribute__((interrupt));
// void ADC1_2_IRQHandler(void)
// {
    
//     uint16_t value = ADC_RDATAR;
//     adc_val = value;

//     convertedValues[currentConversion] = value;
//     // char buf[64];
//     // sprintf(buf, "%u\r\n", adc_val);

//     // print_string(buf);

//     currentConversion++;

//     if (currentConversion >= channelCount){
//         currentConversion = 0;
//         conv_cmplt=1;
//     }
// }










