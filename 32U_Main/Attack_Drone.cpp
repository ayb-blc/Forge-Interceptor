#include "Attack_Drone.h"
#include "Globals.h"
#include <RF24.h>

extern RF24 radio;

// SYMA AND BAYANG PROTOCOLS
const uint8_t drone_channels[] = { 0, 10, 20, 30, 40, 50, 60, 70 }; 
int dr_ch_index = 0;

void SetupDroneSentry() {
    radio.powerUp();
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS); // DRONES PREFER LOW SPEED FOR RANGE
    radio.disableCRC();
    radio.openReadingPipe(0, 0xAALL); // WIDE-BAND SNIFFING
    radio.startListening();
}

void RunDroneSentryLoop() {
    // Kanal Hopla
    radio.stopListening();
    radio.setChannel(drone_channels[dr_ch_index]);
    radio.startListening();

    display.clearDisplay();
    display.setCursor(0,0); display.println("DRONE SENTRY");
    display.drawLine(0, 10, 128, 10, SH110X_WHITE);
    display.setCursor(0,20); display.print("Radar Ch: "); display.println(drone_channels[dr_ch_index]);

    unsigned long start = millis();
    bool droneDetected = false;

    while(millis() - start < 150) {
        if(radio.available()) {
            uint8_t buf[32];
            radio.read(buf, 32);

            // --- SIGNATURE ANALYSIS ---
            // Syma X5C protocol header
            // Usually contains specific preamble and checksum.
            // This is a basic "traffic density" check.
            
            // If the packet is populated and specific bytes match (e.g., Syma Checksum)
            // (Here we are simply checking packet length and content)
            if(buf[0] == 0xA5 || buf[0] == 0x5A) { 
                 droneDetected = true;
                 
                 display.fillRect(0, 35, 128, 30, SH110X_WHITE);
                 display.setTextColor(SH110X_BLACK);
                 display.setCursor(20,45); display.println("DRONE WARNING!");
                 display.setTextColor(SH110X_WHITE);
                 display.display();
                 
                 TrapSerial.println("ALERT: Drone Signal Detected!");
                 delay(500); 
            }
        }
    }
    
    if(!droneDetected) {
        display.setCursor(40,45); display.println("[ --- ]");
    }

    display.display();
    dr_ch_index = (dr_ch_index + 1) % sizeof(drone_channels);
}

void StopDroneSentry() {
    radio.powerDown();
}