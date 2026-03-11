#include "Attack_EvilTwin.h"
#include "Globals.h"



bool AskUserStep(String stepTitle, String question) {
    while(digitalRead(BTN_ACTION) == LOW || digitalRead(BTN_DOWN) == LOW);
    delay(100);

    bool answered = false;
    bool answer = false;

    while(!answered) {
        display.clearDisplay();
        
        // Header 
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0); display.println(stepTitle);
        display.drawLine(0, 10, 128, 10, SH110X_WHITE);

        // Question Text
        display.setCursor(0, 25);
        display.println(question);

        // Options 
        display.fillRect(0, 52, 128, 12, SH110X_WHITE); 
        display.setTextColor(SH110X_BLACK); 
        display.setCursor(2, 54); display.print("[OK]:YES");
        display.setCursor(64, 54); display.print("[DOWN]:NO");
        
        display.display();

        // --- KEY CONTROLS ---
        
        // YES (Action Key)
        if(digitalRead(BTN_ACTION) == LOW) {
            answer = true;
            answered = true;
            delay(100); // Debounce
        }

        // NO (Down or Up Key)
        if(digitalRead(BTN_DOWN) == LOW || digitalRead(BTN_UP) == LOW) {
            answer = false;
            answered = true;
            delay(100); // Debounce
        }
    }
    
    while(digitalRead(BTN_ACTION) == LOW || digitalRead(BTN_DOWN) == LOW);
    return answer;
}

// --- MAIN ATTACK FUNCTION ---
void StartEvilTwin(String ssid) {
    
    // --- STEP 1: WIFI PASSWORD QUERY ---
    // First box in the diagram: "WIFI PASS -> YES/NO""
    bool wantWiFi = AskUserStep("STEP 1 / 2", "Page Type:\nREQUEST WIFI\nPASSWORD?");

    if(wantWiFi) {
        TrapSerial.println("CMD:EVIL:" + ssid);
        
        // Information Display
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        display.setCursor(10, 20); display.println("ATTACK STARTED");
        display.setCursor(10, 40); display.println("MOD: WIFI PASS");
        display.display();
        return;
    }

    // --- STEP 2: CLONE/EMAIL QUERY ---
    // If Wifi is set to NO, the flow directs here.
    bool wantEmail = AskUserStep("STEP 2 / 2", "Page Type:\nEMAIL / CLONE\nEnable?");

    if(wantEmail) {
        TrapSerial.println("CMD:EVIL_EMAIL:" + ssid);
        
        // Information Display
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        display.setCursor(10, 20); display.println("ATTACK STARTED");
        display.setCursor(10, 40); display.println("MOD: EMAIL/KLON");
        display.display();
        return; 
    }

    // --- IF NONE SELECTED ---
    // Both set to NO -> Cancel.
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setCursor(20, 30); display.println("CANCELLED");
    display.display();
    delay(1000);
}

void StopEvilTwin() {
    TrapSerial.println("STOP");
    display.clearDisplay();
    display.setCursor(30, 30); display.println("STOPPED");
    display.display();
    delay(1000);
}