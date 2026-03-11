#include "Attack_Spectrum.h"
#include "Globals.h"
#include <SPI.h>
#include <RF24.h>

byte signalValues[128]; 
const int num_channels = 80; // Covers Wi-Fi frequencies (2.400 - 2.480 GHz)

void SetupSpectrum() {
    // 1. First, test the presence of the chip
    if(!radio.begin()) {
        display.clearDisplay();
        display.setCursor(0,20); display.println("NRF24 HARDWARE ERROR!");
        display.setCursor(0,35); display.println("Check the cables");
        display.display();
        TrapSerial.println("ERROR: Communication with NRF24 could not be established.");
        delay(3000); 
        return;
    }
    
    // 2. If the chip is present, proceed to settings
    radio.setAutoAck(false);
    radio.startListening();
    radio.stopListening();
    radio.setDataRate(RF24_1MBPS); 
    radio.setPALevel(RF24_PA_MIN); 
    
    memset(signalValues, 0, sizeof(signalValues));
}
void RunSpectrumLoop() {
    // Scan 80 Channels
    for (int i = 0; i < num_channels; i++) {
        radio.setChannel(i);
        radio.startListening();
        
        delayMicroseconds(130); 
        
        // --- HERE IS THE REAL SPECTRUM LOGIC ---
        // Instead of measuring just once, we quickly measure 10 times on the channel to find the "Density.
        int hitCount = 0;
        for(int j = 0; j < 10; j++) {
            if(radio.testRPD()) { // If the signal is stronger than -64dBm
                hitCount++;
            }
            delayMicroseconds(10);
        }
        
        radio.stopListening();
        
        int x = map(i, 0, num_channels, 0, 127);
        
        // Signal Rise / Fall Animation (Original Spectrum Feel)
        if (hitCount > 0) {
            signalValues[x] += (hitCount * 3); // 
            if(signalValues[x] > 50) signalValues[x] = 40; 
        } else {
            if(signalValues[x] > 0) signalValues[x] -= 2; 
        }
    }

    // Ekranı Çiz
    display.clearDisplay();
    display.setCursor(0,0); display.println("2.4GHz SPECTRUM");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE); 
    
    //  Draw Bouncing Bars (Bottom to Top)
    for (int x = 0; x < 128; x++) {
        if (signalValues[x] > 0) {
            display.drawLine(x, 63, x, 63 - signalValues[x], SH110X_WHITE);
        }
    }
    
    //Wi-Fi Ch 1, 6, 11
    display.setTextSize(1);
    display.setCursor(0, 54); display.print("1");
    display.setCursor(60, 54); display.print("6");
    display.setCursor(110, 54); display.print("11");
    
    display.display();
}

void StopSpectrum() {
    radio.powerDown();
}