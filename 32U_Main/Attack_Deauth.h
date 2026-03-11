#ifndef ATTACK_DEAUTH_H
#define ATTACK_DEAUTH_H
#include <Arduino.h>

void StartDeauth(uint8_t* targetBSSID, int channel);
void RunDeauthLoop();
void StopDeauth();

#endif