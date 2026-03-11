#ifndef ATTACK_RFID_H
#define ATTACK_RFID_H
#include <Arduino.h>

// The controller only issues commands; the Trap handles the heavy lifting.
void StartRFIDScan();
void StopRFIDScan();

#endif