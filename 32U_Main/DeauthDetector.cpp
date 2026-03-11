#include "DeauthDetector.h"
#include "Globals.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include <RF24.h> 

// Counters
int deauthPacketCount = 0;
int probePacketCount = 0;
String attackStatus = "SAFE";

// --- PROMISCUOUS CALLBACK  ---
// IRAM_ATTR: This places the function in the fastest part of the RAM, preventing crashes.
void IRAM_ATTR snifferResult(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;

    const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t *frame = pkt->payload;

    // Frame Control
    // 0xA0 veya 0xC0 = Deauthentication / Disassociation
    if (frame[0] == 0xA0 || frame[0] == 0xC0) {
        deauthPacketCount++;
    }
    // 0x40 = Probe Request
    else if (frame[0] == 0x40) {
        probePacketCount++;
    }
}

void SetupDetector() {
    // --- SHUT DOWN NRF RADIO ---
    // While WiFi Radar is starting, NRF must sleep; otherwise, the voltage will drop.
    if(radio.isChipConnected()) {
        radio.powerDown(); 
        delay(50); 
    }

    // WiFi Settings
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Turn on Promiscuous Mode
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&snifferResult);
    
    deauthPacketCount = 0;
    probePacketCount = 0;
    attackStatus = "SAFE";
}

void RunDeauthDetector() {
    static uint32_t lastReset = 0;
    
    int ch = (millis() / 200) % 13 + 1; 
    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);

    // --- ATTACK ANALYSIS ---
    if (deauthPacketCount > 10) {
        attackStatus = "ATTACK IN PROGRESS!";
    } else {
        attackStatus = "SAFE";
    }

    // --- Display ---
    display.clearDisplay();
    
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0); 
    display.print("WIFI RADAR  CH: "); 
    display.println(ch);
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);

    // Status Indicator
    display.setCursor(0, 20); 
    display.print("STATUS: "); 
    if(deauthPacketCount > 10) {
        display.setTextColor(SH110X_BLACK, SH110X_WHITE); 
    }
    display.println(attackStatus);
    display.setTextColor(SH110X_WHITE); 

    // Stats
    display.setCursor(0, 40); display.print("Deauth Pkts: "); display.println(deauthPacketCount);
    display.setCursor(0, 52); display.print("Probe Pkts:  "); display.println(probePacketCount);

    // Activity Bar
    int barWidth = map(deauthPacketCount, 0, 50, 0, 128);
    if(barWidth > 128) barWidth = 128;
    display.fillRect(0, 62, barWidth, 2, SH110X_WHITE);

    display.display();


    if (millis() - lastReset >= 1000) {
        deauthPacketCount = 0;
        probePacketCount = 0;
        lastReset = millis();
    }
    
    delay(10); 
} 

void StopDetector() {
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_OFF);
}