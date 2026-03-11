#include "HandshakeCatcher.h"
#include "Globals.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <SD.h>
#include <FS.h>

// --- SETTINGS ---
int hc_channel = 1;
unsigned long hc_lastChannelChange = 0;
int eapol_count = 0;
int pmkid_count = 0;

// Hybrid Recording (If No SD, Use RAM)
File pcapFile;
bool useSD = false; 

// RAM Buffer Settings (Max 30KB for ESP32 RAM limit)
#define MAX_PCAP_SIZE 30000 
uint8_t pcap_buffer[MAX_PCAP_SIZE];
size_t pcap_len = 0;
bool bufferFull = false;

// Web Server (Runs in download mode only)
WebServer pcapServer(80);
bool isServing = false;

// PCAP Global Headed
const uint8_t pcap_global_header[] = {
    0xD4, 0xC3, 0xB2, 0xA1, 0x02, 0x00, 0x04, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0xFF, 0xFF, 0x00, 0x00, 0x69, 0x00, 0x00, 0x00
};

// --- PROMISCUOUS CALLBACK ---
void IRAM_ATTR wifi_sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (isServing) return; 
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) return;

    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    uint8_t *frame = pkt->payload;
    uint16_t len = pkt->rx_ctrl.sig_len;
    bool isInteresting = false;

    // EAPOL / PMKID Search (Simple Filter)
    if (type == WIFI_PKT_DATA && len > 32) {
        for(int i = 24; i < 40; i++) {
            if(frame[i] == 0x88 && frame[i+1] == 0x8E) {
                isInteresting = true;
                eapol_count++;
                break;
            }
        }
    }
    if (type == WIFI_PKT_MGMT && len > 40) {
        if(frame[0] == 0x80) { 
            for(int i = 36; i < len - 2; i++) {
                if(frame[i] == 0x30 && frame[i+1] <= (len - i)) { 
                    isInteresting = true; 
                    pmkid_count++;
                    break;
                }
            }
        }
    }

    // /SAVE PACKET (SD or RAM)
    if (isInteresting && !bufferFull) {
        uint32_t microSeconds = micros();
        uint32_t seconds = millis() / 1000;

        uint8_t pkt_header[16];
        pkt_header[0] = seconds & 0xFF; pkt_header[1] = (seconds >> 8) & 0xFF; 
        pkt_header[2] = (seconds >> 16) & 0xFF; pkt_header[3] = (seconds >> 24) & 0xFF;
        pkt_header[4] = microSeconds & 0xFF; pkt_header[5] = (microSeconds >> 8) & 0xFF; 
        pkt_header[6] = (microSeconds >> 16) & 0xFF; pkt_header[7] = (microSeconds >> 24) & 0xFF;
        pkt_header[8] = len & 0xFF; pkt_header[9] = (len >> 8) & 0xFF; 
        pkt_header[10] = (len >> 16) & 0xFF; pkt_header[11] = (len >> 24) & 0xFF;
        pkt_header[12] = len & 0xFF; pkt_header[13] = (len >> 8) & 0xFF; 
        pkt_header[14] = (len >> 16) & 0xFF; pkt_header[15] = (len >> 24) & 0xFF;

        if (useSD && pcapFile) {
            pcapFile.write(pkt_header, 16);
            pcapFile.write(frame, len);
            pcapFile.flush();
        } else {
            // Write to RAM
            if (pcap_len + 16 + len < MAX_PCAP_SIZE) {
                memcpy(&pcap_buffer[pcap_len], pkt_header, 16); pcap_len += 16;
                memcpy(&pcap_buffer[pcap_len], frame, len); pcap_len += len;
            } else {
                bufferFull = true; // RAM Full!
            }
        }
    }
}

void SetupHandshakeCatcher() {
    hc_channel = 1;
    eapol_count = 0;
    pmkid_count = 0;
    pcap_len = 0;
    bufferFull = false;
    isServing = false;
    
    // SD Card Control
    if(sdAvailable) {
        useSD = true;
        pcapFile = SD.open("/capture.pcap", FILE_WRITE);
        if(pcapFile) {
            pcapFile.write(pcap_global_header, 24);
            pcapFile.flush();
        }
    } else {
        useSD = false;
        memcpy(pcap_buffer, pcap_global_header, 24);
        pcap_len = 24;
    }

    // Start Sniffer
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    wifi_promiscuous_filter_t filter;
    filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_cb);
    esp_wifi_set_channel(hc_channel, WIFI_SECOND_CHAN_NONE);
    
    hc_lastChannelChange = millis();
}

// --- SERVER MODE (TRANSFERRING FILE TO PHONE) ---
void TriggerPcapDownload() {
    if(useSD) return; // No server needed if SD card is present

    isServing = true;
    esp_wifi_set_promiscuous(false); 
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PWN_CATCHER"); 

    pcapServer.on("/pwn", HTTP_GET, []() {
        pcapServer.sendHeader("Content-Disposition", "attachment; filename=\"handshakes.pcap\"");
        pcapServer.send_P(200, "application/vnd.tcpdump.pcap", (const char*)pcap_buffer, pcap_len);
    });
    
    pcapServer.begin();
}

void RunHandshakeCatcherLoop() {
    if(isServing) {
        pcapServer.handleClient();
        display.clearDisplay();
        display.setCursor(0,0); display.println("PCAP FILE READY");
        display.drawLine(0, 10, 128, 10, SH110X_WHITE);
        display.setCursor(0,20); display.println("1. Connect to Network:");
        display.setCursor(10,30); display.println("PWN_CATCHER");
        display.setCursor(0,40); display.println("2. Type in Browser:");
        display.setCursor(10,50); display.println("192.168.4.1/pwn");
        display.display();
        return;
    }

    if(millis() - hc_lastChannelChange > 500) {
        hc_channel++;
        if(hc_channel > 13) hc_channel = 1;
        esp_wifi_set_channel(hc_channel, WIFI_SECOND_CHAN_NONE);
        hc_lastChannelChange = millis();
    }

    display.clearDisplay();
    display.setCursor(0,0); display.println("PWN CATCHER (Sniffing)");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    
    display.setTextSize(2);
    display.setCursor(25, 15);
    if(eapol_count > 0 || pmkid_count > 0) display.print("(o_O)"); 
    else if((millis() / 1000) % 2 == 0) display.print("(^‿^)"); 
    else display.print("(-‿-)"); 
    
    display.setTextSize(1);
    display.setCursor(0, 35); display.print("EAPOL: "); display.print(eapol_count);
    display.setCursor(64, 35); display.print("PMKID: "); display.print(pmkid_count);
    
    display.setCursor(0, 45); 
    if(useSD) display.print("Record: SD Card");
    else if(bufferFull) display.print("RAM FULL! Download!");
    else { display.print("RAM Boyut: "); display.print(pcap_len / 1024); display.print("KB"); }
    
    display.setCursor(0, 56); 
    if(!useSD) display.print("[UP]Download [DOWN]Exit");
    else display.print("[DOWN] Exit");
    
    display.display();
}

void StopHandshakeCatcher() {
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
    esp_wifi_stop();
    WiFi.mode(WIFI_OFF);
    
    if(isServing) {
        pcapServer.stop();
        isServing = false;
    }
    if(useSD && pcapFile) {
        pcapFile.close();
    }
}