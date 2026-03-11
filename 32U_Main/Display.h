#ifndef DISPLAY_H
#define DISPLAY_H
#include "Globals.h" // Add Globals so it recognizes the display object.

void SetupDisplay();
void DrawMainMenu(const char* items[], int count);
void DrawWiFiList(int nNetworks);
void DrawAttackScreen(String infoText);

#endif