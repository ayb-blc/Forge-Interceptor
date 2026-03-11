#include "Display.h"

/// --- GLOBAL VARIABLES TO BE ADDED ---
int menuScrollOffset = 0; 
const int MAX_VISIBLE_ITEMS = 4; 

void SetupDisplay() {
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, HIGH);
    delay(10);
    digitalWrite(OLED_RST, LOW);
    delay(10);
    digitalWrite(OLED_RST, HIGH);
    delay(10);

    // Start
    if(!display.begin(0, true)) { 
        Serial.println("Display Not Found");
        for(;;); 
    }
    
    display.display(); 
    delay(1000);       
    display.clearDisplay(); 
    display.display();      
    
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);
    display.println("System Booting...");
    display.display(); 
    delay(500);
}

void DrawMainMenu(const char* items[], int count) {
    display.clearDisplay();
    
    // --- HEADER ---
    display.setCursor(0, 0);
    display.println("M3RG3N TOOLS");
    
    // SD Card
    display.setCursor(95, 0); 
    display.print(sdAvailable ? "[SD]" : "[NO]");
    
    display.drawLine(0, 9, 128, 9, SH110X_WHITE);

    // ---SCROLL LOGIC ---
    if (selectedIndex >= menuScrollOffset + MAX_VISIBLE_ITEMS) {
        menuScrollOffset = selectedIndex - MAX_VISIBLE_ITEMS + 1;
    } 
    else if (selectedIndex < menuScrollOffset) {
        menuScrollOffset = selectedIndex;
    }

    // --- LIST ---
    for(int i = 0; i < MAX_VISIBLE_ITEMS; i++) {
        
        int currentItemIndex = menuScrollOffset + i; 
        if(currentItemIndex >= count) break;
        display.setCursor(0, 15 + (i * 12)); 

        if(currentItemIndex == selectedIndex) {
            display.print("> "); 
        } else {
            display.print("  "); 
        }
        
        display.println(items[currentItemIndex]);
    }
    
    display.display();
}

void DrawWiFiList(int nNetworks) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SELECT TARGET:");
    display.drawLine(0, 9, 128, 9, SH110X_WHITE);

    int start = 0;
    if (selectedIndex > 3) start = selectedIndex - 3;

    for (int i = start; i < min(start + 4, nNetworks); i++) {
        display.setCursor(0, 12 + ((i - start) * 10));
        if (i == selectedIndex) {
            display.setTextColor(SH110X_BLACK, SH110X_WHITE);
            display.print(">");
        } else {
            display.setTextColor(SH110X_WHITE);
            display.print(" ");
        }
        String ssid = WiFi.SSID(i);
        if(ssid.length() > 18) ssid = ssid.substring(0, 18);
        display.println(ssid);
    }
    display.display();
}

void DrawAttackScreen(String infoText) {
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0); display.println("!!! ATTACK !!!");
    display.drawLine(0, 9, 128, 9, SH110X_WHITE);
    
    display.setCursor(0, 20);
    display.println(infoText);
    
    if(targetSSID != "") {
        display.setCursor(0, 35);
        display.print("Target: "); display.println(targetSSID.substring(0,10));
    }

    display.setCursor(0, 55);
    display.println("[BTN] EXIT"); 
    display.display();
}