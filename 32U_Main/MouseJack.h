#ifndef MOUSEJACK_H
#define MOUSEJACK_H

#include <Arduino.h>

extern bool mj_targetLocked;
extern uint8_t targetAddr[5];

void SetupMouseJack();
void RunMouseJackScanner();    
void SelectPayloadAndFire();   
void StopMouseJack();          

#endif