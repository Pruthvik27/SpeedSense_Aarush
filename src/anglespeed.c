#include <stdio.h>
#include <stdint.h>

#include "ch32fun.h"
#include "bsp.h"
#include "uart.h"
#include "speed.h"
#include "spi.h"
#include "MT6835.h"

// ============================================================================
// CAN DOUBLE BUFFER
// ============================================================================

volatile CAN_Frame_t tx_buffers[2];
volatile uint8_t active_buffer = 0;

// ============================================================================
// SENSOR VALUES
// ============================================================================

volatile uint8_t global_speed = 0;
volatile int16_t global_angle_x100 = 0;

// ============================================================================
// SENSOR INITIALIZATION
// ============================================================================

void Sensor_Hardware_Init(void)
{
    Speed_Init();
    SPI1_init();
}

// ============================================================================
// SPEED
// ============================================================================

void Poll_Speed(void)
{
    global_speed = Speed_getSpeed();
}

// ============================================================================
// MT6835 ANGLE
// ============================================================================

void Poll_Angle(void)
{
    uint32_t raw_angle_spi = 0;

    if(MT6835_GetRawAngleIfReady(&raw_angle_spi))
    {
        /*
         * This is deliberately the same conversion used by your
         * working right-wheel code.
         *
         * MT6835:
         * 0 .. 2097151 counts = 0 .. 360 degrees
         *
         * Result is degrees * 100.
         */

        int32_t angle_x100 =
            (int16_t)(((uint32_t)raw_angle_spi * 1125) >> 16) - 18000;

        // angle_x100 -= 18000;

        /*
         * Force the result into -18000 ... +18000.
         */

        // if(angle_x100 > 18000)
        //     angle_x100 = 18000;

        // if(angle_x100 < -18000)
        //     angle_x100 = -18000;

        global_angle_x100 = (int16_t)angle_x100;

        /*
         * Start next SPI transaction.
         */
        MT6835_TriggerRead();
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(void)
{
    SystemInit();

    BSP_Init();
    Uart_Init();
    Background_LED_Init();

    Sensor_Hardware_Init();

    // ========================================================================
    // Start MT6835
    // ========================================================================

    MT6835_TriggerRead();

    while(SPI1_IsBusy())
    {
    }

    // Consume first completed transaction.
    uint32_t dummy_angle = 0;

    MT6835_GetRawAngleIfReady(&dummy_angle);

    // Start continuous angle acquisition.
    MT6835_TriggerRead();

    // ========================================================================
    // CAN FRAME
    // ========================================================================

    tx_buffers[0].id  = 0x21;
    tx_buffers[0].dlc = 3;

    tx_buffers[1].id  = 0x21;
    tx_buffers[1].dlc = 3;

    tx_buffers[0].data.WAL_SPEED_0X21.speed = 0;
    tx_buffers[0].data.WAL_SPEED_0X21.wheel_angle_left = 0;

    tx_buffers[1].data.WAL_SPEED_0X21.speed = 0;
    tx_buffers[1].data.WAL_SPEED_0X21.wheel_angle_left = 0;

    uint32_t last_sensor_poll = 0;
    uint32_t last_uart = 0;

    char buffer[128];

    // ========================================================================
    // MAIN LOOP
    // ========================================================================

    while(1)
    {
        // --------------------------------------------------------------------
        // Update sensors
        // --------------------------------------------------------------------

        if(system_ms - last_sensor_poll >= 1)
        {
            last_sensor_poll = system_ms;

            Poll_Speed();
            Poll_Angle();

            // ---------------------------------------------------------------
            // Update inactive CAN buffer
            // ---------------------------------------------------------------

            uint8_t write_buffer = active_buffer ^ 1;

            tx_buffers[write_buffer]
                .data.WAL_SPEED_0X21.speed =
                global_speed;

            tx_buffers[write_buffer]
                .data.WAL_SPEED_0X21.wheel_angle_left =
                global_angle_x100;

            active_buffer ^= 1;
        }

        // --------------------------------------------------------------------
        // UART DEBUG
        // --------------------------------------------------------------------

        // if(system_ms - last_uart >= 100)
        // {
        //     last_uart = system_ms;

        //     int16_t whole =
        //         global_angle_x100 / 100;

        //     int16_t fraction =
        //         global_angle_x100 % 100;

        //     if(fraction < 0)
        //         fraction = -fraction;

        //     sprintf(
        //         buffer,
        //         "Speed=%u | Angle=%d.%02d | Freq=%lu\r\n",
        //         global_speed,
        //         whole,
        //         fraction,
        //         Speed_getFrequency()
        //     );

        //     print_string(buffer);
        }
    }
}

// ============================================================================
// CAN SYNC ISR
// ============================================================================

// ============================================================================
// CAN SYNC RX INTERRUPT
// ============================================================================

void USB_LP_CAN1_RX0_IRQHandler(void)
__attribute__((interrupt));

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CAN_Frame_t rx_frame;

    if(CAN1->RFIFO0 & CAN_RFIFO0_FMP0)
    {
        // Receive the incoming CAN frame
        CAN_Receive(&rx_frame, FIFO);

        // CAN communication is alive
        last_sync_time = system_ms;

        // Transmit the latest sensor data
        CAN_Transmit(
            (CAN_Frame_t *)&tx_buffers[active_buffer]
        );
    }
}





