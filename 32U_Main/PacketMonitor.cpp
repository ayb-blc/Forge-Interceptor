#include "PacketMonitor.h"
#include "Globals.h"
#include <WiFi.h>
#include "esp_wifi.h"
#include <RF24.h> 

// Graph data
int graphData[128]; 
int packetRate = 0;

void countPackets(void* buf, wifi_promiscuous_pkt_type_t type) {
    packetRate++; 
}

void SetupMonitor() {
    // --- CLOSE NRF ---
    if(radio.isChipConnected()) {
        radio.powerDown(); 
    }
    delay(50); 


    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&countPackets);
    
    for(int i=0; i<128; i++) graphData[i] = 0;
}

void RunPacketMonitor() {
    int ch = random(1, 14);
    esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);

    for (int i = 0; i < 127; i++) graphData[i] = graphData[i+1];
    
    int val = packetRate / 2; 
    if (val > 60) val = 60;
    graphData[127] = val;

    display.clearDisplay();
    display.setCursor(0,0); 
    display.print("WIFI MON CH:"); display.print(ch);
    display.setCursor(0,10); display.print("Pkts:"); display.print(packetRate);
    
    for (int x = 0; x < 128; x++) {
        if(graphData[x] > 0) display.drawLine(x, 64, x, 64 - graphData[x], SH110X_WHITE);
    }
    display.display();

    packetRate = 0;
    delay(100); 
}

void StopMonitor() {
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_OFF); /// Completely turn off WiFi
    // Since NRF will be reopened on exit, we don't touch it here;
    // when another tool is selected in the main loop, that tool will open it.
}