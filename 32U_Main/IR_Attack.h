#ifndef IR_ATTACK_H
#define IR_ATTACK_H

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

extern IRsend irsend;
extern IRrecv irrecv;

void SetupIR();
void FireTVBGone();
void RunIRLearner();
void TestIRLed(); 

#endif