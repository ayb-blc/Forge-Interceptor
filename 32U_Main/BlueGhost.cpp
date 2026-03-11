#include "BlueGhost.h"
#include "Globals.h" 
#include <NimBLEDevice.h>

int scanTime = 5; 
BLEScan* pBLEScan;
bool skimmerFound = false;
String foundMac = "";

// Skimmer Prefix's
const char* skimmerPrefixes[] = {
    "98:D3:31", "20:16:08", "00:14:03", "20:15:05", "00:0E:0E"
};
int numPrefixes = 5;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice* advertisedDevice) {
        String devMac = advertisedDevice->getAddress().toString().c_str();
        devMac.toUpperCase(); 

        for (int i = 0; i < numPrefixes; i++) {
            if (devMac.startsWith(skimmerPrefixes[i])) {
                skimmerFound = true;
                foundMac = devMac;
                if(advertisedDevice->getScan() != nullptr) {
                    advertisedDevice->getScan()->stop(); 
                }
            }
        }
    }
};

void SetupBlueGhost() {
    if(!NimBLEDevice::getInitialized()) NimBLEDevice::init("");
    
    pBLEScan = BLEDevice::getScan(); 
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); 
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  
}

void RunBlueGhost() {
    skimmerFound = false;
    foundMac = "";

    display.clearDisplay();
    display.setCursor(0,0); display.println("BLUE GHOST");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    display.setCursor(0,20); display.println("Scanning (5 sec)...");
    display.display();

    // Taramayı Başlat
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    
    display.clearDisplay();
    if (skimmerFound) {
        display.fillRect(0, 0, 128, 64, SH110X_WHITE); 
        display.display(); delay(100); display.clearDisplay();
        
        display.setCursor(10,10); display.setTextSize(2); display.println("DANGER!");
        display.setTextSize(1);
        display.setCursor(0,35); display.println("Skimmer Found:");
        display.setCursor(0,50); display.println(foundMac);
        TrapSerial.println("ALERT: Skimmer Found -> " + foundMac);
    } 
    else {
        display.setCursor(0,10); display.setTextSize(2); display.println("CLEAN");
        display.setTextSize(1);
        display.setCursor(0,35); display.print(foundDevices.getCount()); display.println(" Device");
        display.setCursor(0,50); display.println("No Threat.");
    }
    
    // --- Exit Logic ---
    display.setCursor(0,56); display.print("[DOWN] Exit [UP] Repeat");
    display.display();
    pBLEScan->clearResults(); 
    
    while(true) {
        if(digitalRead(BTN_DOWN) == LOW) {
            break; 
        }
        if(digitalRead(BTN_UP) == LOW) {
            break; 
        }
        delay(50);
    }
}


void StopBlueGhost() {
    if(pBLEScan != nullptr) {
        pBLEScan->stop(); 
        pBLEScan->clearResults(); 
    }
}