#include "Attack_KeySniffer.h"
#include "Globals.h"
#include <RF24.h>

extern RF24 radio;

// Channels to Scan (Logitech/Microsoft usually prefer these)
const uint8_t ks_channels[] = { 3, 6, 9, 12, 18, 24, 26, 30, 40, 50, 60, 70, 78 };
int ks_ch_index = 0;

void SetupKeySniffer() {
    radio.powerUp();
    radio.setAutoAck(false);
    radio.setPayloadSize(32); // Max packet size
    radio.setDataRate(RF24_2MBPS);
    radio.disableCRC(); // Disabling CRC to view raw data
    
    // Settings similar to Promiscuous Mode (Address filtering is difficult to loosen,
    // but we can listen for known preambles. We are performing a simple scan here)
    radio.openReadingPipe(1, 0xAALL); // Dummy address (Full sniffing requires lower-level register settings)
    radio.startListening();
}

void RunKeySnifferLoop() {
    // Change Channel
    radio.stopListening();
    radio.setChannel(ks_channels[ks_ch_index]);
    radio.startListening();
    
    // Print info to screen
    display.clearDisplay();
    display.setCursor(0,0); display.println("KEYBOARD HUNTER");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    display.setCursor(0,20); display.print("Scanning Ch: "); display.println(ks_channels[ks_ch_index]);
    
    unsigned long start = millis();
    bool found = false;
    
    while(millis() - start < 100) { // Listen 100ms
        if(radio.available()) {
            uint8_t buf[32];
            radio.read(buf, 32);
            
            // --- ANALYSIS ---
            // Microsoft Keyboards usually start with 0x0A or 0x0C (Device Type)
            // Logitech Unifying packets contain specific patterns if unencrypted.

            // Simple Signature Check (Example: Microsoft Wireless Desktop)
            // Note: This part is just an example; actual signature analysis is complex.
            if(buf[0] == 0x0A && buf[4] == 0xCD) { // EXAMPLE SIGNATURE
                found = true;
                display.fillRect(0, 35, 128, 30, SH110X_WHITE);
                display.setTextColor(SH110X_BLACK);
                display.setCursor(10,45); display.println("MS KEYBOARD!");
                display.setTextColor(SH110X_WHITE);
                display.display();
                TrapSerial.println("ALERT: Vulnerable Keyboard Found!");
                delay(1000);
            }
        }
    }
    
    if(!found) {
        display.setCursor(0,40); display.println("... LISTENING ...");
    }
    
    display.display();
    ks_ch_index = (ks_ch_index + 1) % sizeof(ks_channels);
}

void StopKeySniffer() {
    radio.powerDown();
}