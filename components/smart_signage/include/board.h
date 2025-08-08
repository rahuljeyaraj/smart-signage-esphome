#pragma once

// #ifdef ARDUINO_XIAO_ESP32C3
// // Potentiometers
// #define POT0_ADC_PIN GPIO_NUM_2
// #define POT1_ADC_PIN GPIO_NUM_3

// // Buttons
// #define BUTTON0_PIN GPIO_NUM_4

// // LEDs
// #define LED0_PIN GPIO_NUM_5

// // I2C
// #define I2C_SDA_PIN GPIO_NUM_6
// #define I2C_SCL_PIN GPIO_NUM_7

// // I2S
// #define I2S_LRCK_PIN GPIO_NUM_8
// #define I2S_BCLK_PIN GPIO_NUM_9
// #define I2S_DATA_PIN GPIO_NUM_10

// // UART
// #define RADAR_RX_PIN GPIO_NUM_20
// #define RADAR_TX_PIN GPIO_NUM_21

// #elif defined(ARDUINO_XIAO_ESP32S3)

#define POT0_ADC_PIN GPIO_NUM_1
#define POT1_ADC_PIN GPIO_NUM_2

#define BUTTON0_PIN GPIO_NUM_3

#define LED0_PIN GPIO_NUM_4

#define I2C_SDA_PIN GPIO_NUM_5
#define I2C_SCL_PIN GPIO_NUM_6

#define I2S_LRCK_PIN GPIO_NUM_7
#define I2S_BCLK_PIN GPIO_NUM_8
#define I2S_DATA_PIN GPIO_NUM_9

#define RADAR_RX_PIN GPIO_NUM_44
#define RADAR_TX_PIN GPIO_NUM_43

// #else
//   #error "Define one of ARDUINO_XIAO_ESP32C3 or ARDUINO_XIAO_ESP32S3"
// #endif