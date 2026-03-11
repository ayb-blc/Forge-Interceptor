#include "Attack_RFID.h"
#include "Globals.h"

void StartRFIDScan() {
    TrapSerial.println("CMD:RFID");
    TrapSerial.flush();
}

void StopRFIDScan() {
    TrapSerial.println("STOP");
    TrapSerial.flush();
}