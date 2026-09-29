#ifndef BSP_H
#define BSP_H

#include "ch32fun.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ==============================================================================
// CAN BUS MACROS
// ==============================================================================
#define SYNC_PULSE      0x000
#define FIFO            0
// Calculated for 72MHz APB1 Clock (144MHz System Clock / 2)
#define BAUD_500kbps    ( (2 << 24) | (13 << 16) | (2 << 20) | (7) )

// ==============================================================================
// THE MEGA STRUCT: TELEMETRY PAYLOAD MAPPINGS
// ==============================================================================
typedef struct {
    uint32_t id;
    uint8_t  dlc;
    union {
        // The Raw Memory Doors (For CAN Hardware)
        uint8_t  bytes[8];
        uint16_t halfwords[4];
        uint32_t words[2];

        // --------------------------------------------------
        // VEHICLE DYNAMICS (NODE 1)
        // --------------------------------------------------
        struct __attribute__((packed)) {
            int16_t accel_x; // Bytes 1-2
            int16_t accel_y; // Bytes 3-4
            int16_t accel_z; // Bytes 5-6
        } ACCEL_0X01;

        struct __attribute__((packed)) {
            int16_t gyro_x;  // Bytes 1-2
            int16_t gyro_y;  // Bytes 3-4
            int16_t gyro_z;  // Bytes 5-6
        } GYRO_0x11;

        struct __attribute__((packed)) {
            uint8_t speed;              // Byte 1
            int16_t wheel_angle_left;   // Bytes 2-3
        } WAL_SPEED_0X21;

        struct __attribute__((packed)) {
            int16_t wheel_angle_right;  // Bytes 1-2
            int16_t steering_angle;     // Bytes 3-4
        } WAR_STEERING_0X31;

        struct __attribute__((packed)) {
            uint16_t front_bpt;         // Bytes 1-2
            uint8_t  bias_percentage;   // Byte 3
        } BPTF_BIAS_0X41;

        struct __attribute__((packed)) {
            uint16_t rear_bpt;          // Bytes 1-2
        } BPTR_0X51;

        // --------------------------------------------------
        // POWERTRAIN (NODE 2)
        // --------------------------------------------------
        struct __attribute__((packed)) {
            uint8_t coolant_flow;       // Byte 1
            uint8_t fuel_flow;          // Byte 2
            int16_t inlet_temp;         // Bytes 3-4
            int16_t outlet_temp;        // Bytes 5-6
        } COOLING_0X02;

        struct __attribute__((packed)) {
            uint8_t  battery_v;         // Byte 1 (e.g., 142 = 14.2V)
            uint8_t  gear;              // Byte 2
            int16_t  ect;               // Bytes 3-4 (Engine Coolant Temp)
            uint16_t rpm;               // Bytes 5-6
        } POWERTRAIN_0X12;

    } data;
} CAN_Frame_t;

// ==============================================================================
// GLOBAL TIMERS
// ==============================================================================
extern volatile uint32_t last_sync_time;
extern volatile uint32_t system_ms;

// ==============================================================================
// UNIFIED FUNCTION PROTOTYPES
// ==============================================================================
// Framework Initialization
void BSP_Init(void);
void Background_LED_Init(void);

// CAN Functions
void CAN_Receive(CAN_Frame_t *frame, uint8_t fifo);
int  CAN_Transmit(const CAN_Frame_t *frame);

#endif // BSP_H