#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PIN DEFINITIONS ---

// OLED Display (SPI)
#define OLED_MOSI  23
#define OLED_CLK   18
#define OLED_DC    2
#define OLED_RST   4
#define OLED_CS    5

// SD Card
#define SD_CS_PIN  15

// Buttons
#define BTN_DOWN   13
#define BTN_UP     27
#define BTN_ACTION 14

// UART (Communication with 32S)
#define RX_PIN 33 
#define TX_PIN 32

// NRF24L01 PINS
#define NRF_CE_PIN   25
#define NRF_CSN_PIN  26

#endif