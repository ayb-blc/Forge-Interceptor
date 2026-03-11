#include "IR_Attack.h"
#include "Globals.h"
#include <IRutils.h>

// Pin Settings
const uint16_t kIrLedPin = 22;  
const uint16_t kRecvPin = 15;   

IRsend irsend(kIrLedPin);
IRrecv irrecv(kRecvPin);
decode_results results;
bool irInitialized = false;

// Protocol Types and Structure
enum ProtocolType { P_NEC, P_SAMSUNG, P_SONY, P_PANASONIC, P_RC5, P_RC6 };
struct PowerCode {
    ProtocolType type;
    uint64_t data;
    uint16_t bits;
    const char* name; 
};

// Database
PowerCode database[] = {
    {P_SAMSUNG, 0xE0E019E6, 32, "Samsung (OFF)"},
    {P_SAMSUNG, 0xE0E040BF, 32, "Samsung (Mix)"},
    {P_NEC, 0x20DF10EF, 32, "LG TV"},
    {P_SONY, 0xF50, 12, "Sony (OFF)"},
    {P_PANASONIC, 0x40040100BCBD, 48, "Panasonic"},
};
int dbSize = sizeof(database) / sizeof(database[0]);

void SetupIR() {
    if (irInitialized) return;
    
    pinMode(kIrLedPin, OUTPUT);
    digitalWrite(kIrLedPin, LOW); 
    
    irsend.begin(); 
    irrecv.enableIRIn(); 
    irInitialized = true;
}

void FireTVBGone() {
    display.clearDisplay();
    display.setCursor(0,0); display.println("TV-B-GONE PRO");
    display.display();
    
    irrecv.disableIRIn(); 
    
    for (int i = 0; i < dbSize; i++) {
        switch (database[i].type) {
            case P_NEC: irsend.sendNEC(database[i].data, database[i].bits); break;
        }
        delay(500); 
    }
    
    irrecv.enableIRIn(); 
}

void RunIRLearner() {
    // Printing to screen and decoding processes
    if (irrecv.decode(&results)) {
        irrecv.resume();
    }
}

void TestIRLed() {
    // LED Test Code
}