/*
 * PROJE: FORGE-INTERCEPTOR
 * EKRAN: 1.3" OLED (SPI)
 * BAĞLANTI: S3 <-> 32U (RX:33, TX:32)
 */

#include "Globals.h"
#include "Config.h"
#include "Display.h"
#include "soc/soc.h"             
#include "soc/rtc_cntl_reg.h"    

// --- ATTACK FILES ---
#include "Attack_EvilTwin.h" 
#include "Attack_BLE.h"      
#include "Attack_Beacon.h"   
#include "Attack_Deauth.h"   
#include "Attack_Probe.h"    
#include "MouseJack.h"
#include "IR_Attack.h"
#include "BlueGhost.h"
#include "DeauthDetector.h"
#include "PacketMonitor.h"
#include "Attack_Spectrum.h"
#include "Attack_KeySniffer.h"
#include "Attack_Drone.h"
#include "HandshakeCatcher.h"
#include "Attack_RFID.h"

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &SPI, OLED_DC, OLED_RST, OLED_CS);
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
HardwareSerial TrapSerial(2); 

// --- GLOBAL VARIABLES ---
bool isAttacking = false;
bool sdAvailable = false;
String targetSSID = "";
String capturedLogs = "";
int selectedIndex = 0;
int attackType = 0; 
int networkCount = 0;

uint8_t targetBSSID[6];
int targetChannel;

SystemState currentState = MAIN_MENU;

// --- MENU ---
const char* mainMenuItems[] = {
    "1. Evil Twin", 
    "2. BLE Spam", 
    "3. Beacon Flood", 
    "4. Deauth (DoS)", 
    "5. Probe Flood",
    "6. MouseJack",
    "7. IR Blaster",
    "8. BlueGhost",
    "9. WiFi Radar",
    "10. Traffic Monitor",
    "11. RF Spectrum",
    "12. KeySniffer (Audit)",
    "13. Drone Sentry",
    "14. Pwn Catcher",
    "15. RFID Scanner"
};
int menuCount = 15;

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

    Serial.begin(115200);
    TrapSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); 

    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_ACTION, INPUT_PULLUP);

    if(radio.begin()) {
        Serial.println("NRF24 Initialized!");
        radio.setPALevel(RF24_PA_LOW); 
    } else {
        Serial.println("NRF24 NOT FOUND!");
    }

    SetupDisplay(); 

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void loop() {
    // --- LISTEN FOR DATA COMING FROM THE TRAP (32S) ---
    if (TrapSerial.available()) {
        String data = TrapSerial.readStringUntil('\n');
        data.trim();
        
        if (data.startsWith("LOG:")) {
            display.clearDisplay();
            display.fillRect(0, 0, 128, 64, SH110X_WHITE); 
            display.display();
            delay(100);
            display.clearDisplay();
            display.setTextColor(SH110X_WHITE);
            
            if(data.startsWith("LOG:RFID:")) {
                String uid = data.substring(9);
                display.setCursor(0, 15); display.setTextSize(1); display.println("NEW CARD!");
                display.setCursor(0, 35); display.setTextSize(2); display.println(uid);
            } else {
                display.setCursor(10, 20); display.setTextSize(2); display.println("CAPTURED!");
                display.setCursor(10, 45); display.setTextSize(1); display.println("SD RECORD OK");
            }
            display.display();
            delay(1200);
        }
    }

    switch(currentState) {
        // --- 1. MAIN MENU ---
        case MAIN_MENU:
            DrawMainMenu(mainMenuItems, menuCount);
            
            if(digitalRead(BTN_DOWN) == LOW) { selectedIndex = (selectedIndex + 1) % menuCount; delay(200); }
            if(digitalRead(BTN_UP) == LOW)   { selectedIndex = (selectedIndex - 1 + menuCount) % menuCount; delay(200); }
            
            if(digitalRead(BTN_ACTION) == LOW) {
                attackType = selectedIndex + 1;

                if(attackType == 1 || attackType == 4) { 
                    display.clearDisplay(); 
                    display.setCursor(0,20); display.println("SCANNING NETWORKS..."); 
                    display.display();
                    
                    networkCount = WiFi.scanNetworks();
                    currentState = WIFI_SCAN_LIST; 
                    selectedIndex = 0; 
                }
                else {
                    if(attackType == 2) StartBLESpam();
                    if(attackType == 3) StartBeaconFlood();
                    if(attackType == 5) StartProbeFlood();
                    if(attackType == 6) SetupMouseJack();
                    if(attackType == 7) SetupIR();
                    if(attackType == 8) SetupBlueGhost();
                    if(attackType == 9) SetupDetector();
                    if(attackType == 10) SetupMonitor();
                    if(attackType == 11) SetupSpectrum();
                    if(attackType == 12) SetupKeySniffer();
                    if(attackType == 13) SetupDroneSentry();
                    if(attackType == 14) SetupHandshakeCatcher();
                    if(attackType == 15) StartRFIDScan(); 
                    currentState = ATTACK_RUNNING;
                }
                delay(300);
            }
            break;

        // --- 2. NETWORK PICKLIST ---
        case WIFI_SCAN_LIST:
            DrawWiFiList(networkCount);

            if(digitalRead(BTN_DOWN) == LOW) { selectedIndex = (selectedIndex + 1) % networkCount; delay(150); }
            if(digitalRead(BTN_UP) == LOW)   { selectedIndex = (selectedIndex - 1 + networkCount) % networkCount; delay(150); }
            
            if(digitalRead(BTN_ACTION) == LOW) {
                targetSSID = WiFi.SSID(selectedIndex);
                
                if(attackType == 1) {
                    StartEvilTwin(targetSSID);
                    DrawAttackScreen("HONEYPOT ONLINE\nWaiting for Victim...");
                }
                else if(attackType == 4) { 
                    uint8_t* bssid = WiFi.BSSID(selectedIndex);
                    memcpy(targetBSSID, bssid, 6);
                    targetChannel = WiFi.channel(selectedIndex);
                    StartDeauth(targetBSSID, targetChannel);
                    DrawAttackScreen("DOS ATTACK\nTarget: DOWN");
                }
                currentState = ATTACK_RUNNING;
                delay(500);
            }
            break;

        // --- 3. ATTACK SCREEN ---
        case ATTACK_RUNNING:
            
            // --- REQUIRING SPECIAL OUTPUT/CONTROL ---
            if(attackType == 6) {
                RunMouseJackScanner();
                if(digitalRead(BTN_ACTION) == LOW) {
                    if(mj_targetLocked) { SelectPayloadAndFire(); delay(500); } 
                    else { StopMouseJack(); currentState = MAIN_MENU; attackType = 0; delay(1000); }
                }
            }
            else if(attackType == 7) {
                RunIRLearner(); 
                if(digitalRead(BTN_UP) == LOW) FireTVBGone();
                if(digitalRead(BTN_DOWN) == LOW) { irrecv.disableIRIn(); currentState = MAIN_MENU; attackType = 0; delay(1000); }
            }
            else if(attackType == 8) {
                RunBlueGhost(); 
                if(digitalRead(BTN_DOWN) == LOW) { StopBlueGhost(); currentState = MAIN_MENU; attackType = 0; delay(500); }
                else if(digitalRead(BTN_UP) == LOW) { delay(500); }
            }
            else if(attackType == 14) {
                RunHandshakeCatcherLoop();
                if(digitalRead(BTN_UP) == LOW) { TriggerPcapDownload(); delay(500); }
                if(digitalRead(BTN_DOWN) == LOW) { StopHandshakeCatcher(); currentState = MAIN_MENU; attackType = 0; delay(1000); }
            }
            
            // --- REQUIRING STANDARD OUTPUT (Only with the ACTION Button)---
            else {
                // Screen / Loop Functions
                if(attackType == 1) { delay(100); } // Evil Twin (32S)
                else if(attackType == 2) { RunBLESpamLoop(); DrawAttackScreen("BLE CRASHER\niOS/Android Spam"); }
                else if(attackType == 3) { RunBeaconFloodLoop(); DrawAttackScreen("BEACON STORM\nBROADCASTING FAKE AP"); }
                else if(attackType == 4) { RunDeauthLoop(); }
                else if(attackType == 5) { RunProbeFloodLoop(); DrawAttackScreen("PROBE FLOOD\nNoise Level: MAX"); }
                else if(attackType == 9) { RunDeauthDetector(); }
                else if(attackType == 10) { RunPacketMonitor(); }
                else if(attackType == 11) { RunSpectrumLoop(); }
                else if(attackType == 12) { RunKeySnifferLoop(); }
                else if(attackType == 13) { RunDroneSentryLoop(); }
                else if(attackType == 15) {
                    display.clearDisplay();
                    display.setTextSize(1); 
                    display.setTextColor(SH110X_WHITE);
                    
                    display.setCursor(15, 0); 
                    display.println("[ RFID SKIMMER ]");
                    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
                    
                
                    display.setCursor(0, 15); display.println("1. SCAN CARDS");
                    display.setCursor(0, 25); display.println("2. WiFi: RFID_VAULT");
                    display.setCursor(0, 35); display.println("3. 192.168.4.1/rfid");
                    
                    display.drawLine(0, 50, 128, 50, SH110X_WHITE);
                    display.setCursor(15, 54); 
                    display.println("[ACTION] Check Out");
                    
                    display.display();
                }

                // GENERAL OUTPUT BUTTON (ACTION)
                if(digitalRead(BTN_ACTION) == LOW) {
                    if(attackType == 1) StopEvilTwin();
                    else if(attackType == 2) StopBLESpam();
                    else if(attackType == 3) StopBeaconFlood();
                    else if(attackType == 4) StopDeauth();
                    else if(attackType == 5) StopProbeFlood();
                    else if(attackType == 9) StopDetector();
                    else if(attackType == 10) StopMonitor();
                    else if(attackType == 11) StopSpectrum();
                    else if(attackType == 12) StopKeySniffer();
                    else if(attackType == 13) StopDroneSentry();
                    else if(attackType == 15) {
                        display.clearDisplay();
                        display.setTextSize(1); 
                        display.setTextColor(SH110X_WHITE);
                        
                        
                        display.setCursor(15, 0); 
                        display.println("[ RFID CLONER ]");
                        display.drawLine(0, 10, 128, 10, SH110X_WHITE);
                        
                    
                        display.setCursor(10, 25); 
                        display.println("> WAITING FOR CARD...");
                        display.setCursor(10, 35); 
                        display.println("> 13.56 MHz (NFC)");
                        
                        
                        display.drawLine(0, 52, 128, 52, SH110X_WHITE);
                        display.setCursor(15, 55); 
                        display.println("[ACTION] Check Out");
                        
                        display.display();
                    }

                    currentState = MAIN_MENU;
                    selectedIndex = 0;
                    attackType = 0;
                    delay(1000);
                }
            }
            break;
    }
}