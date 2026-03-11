#ifndef HANDSHAKECATCHER_H
#define HANDSHAKECATCHER_H

#include <Arduino.h>

void SetupHandshakeCatcher();
void RunHandshakeCatcherLoop();
void StopHandshakeCatcher();
void TriggerPcapDownload(); 

#endif