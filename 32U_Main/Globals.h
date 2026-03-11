#ifndef GLOBALS_H
#define GLOBALS_H

#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <SD.h>
#include <RF24.h> 
#include "Config.h"

// --- OBJECT REFERENCE ---
extern Adafruit_SH1106G display;
extern HardwareSerial TrapSerial;

// --- COMMON RADIO OBJECT ---
extern RF24 radio; 

// --- VARIABLE REFERENCES ---
extern bool isAttacking;
extern bool sdAvailable;
extern String targetSSID;
extern String capturedLogs;
extern int selectedIndex;
extern int attackType; 

// Target Information
extern uint8_t targetBSSID[6];
extern int targetChannel;

enum SystemState {
    MAIN_MENU,
    WIFI_SCAN_LIST,
    ATTACK_RUNNING
};
extern SystemState currentState;

#endif