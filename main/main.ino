#include <U8x8lib.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

#define OLED_SCK_PIN 22
#define OLED_SDA_PIN 21

Preferences preferences;

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE, OLED_SCK_PIN, OLED_SDA_PIN);

#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];
U8X8LOG u8x8log;

#define SS_PIN 5
#define RST_PIN 2
#define PIN_BUZZER 4

const int ledPin = 13;
const int resetButtonPin = 32;
const unsigned long holdDuration = 2000;
unsigned long buttonPressTime = 0;
bool isButtonPressed = false;
unsigned long lastClearTime = 0;
const unsigned long clearCooldown = 3000;
const int LED1 = 26;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

byte nuidPICC[4];
byte preNuidPICC[4];
byte pre2NuidPICC[4];
uint32_t counter;

// --- WiFi Access Point Configuration ---
const char* ap_ssid = "ESP32_RFID_Reader";
const char* ap_password = "12345678";

WebServer server(80);

// Global variables for web display
String scanStatus = "Waiting for a card...";
String lastCardDump = "Waiting for a card...\n\nPlace a MIFARE Classic card on the reader\nto view the full memory dump.";

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "0" : "");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  String result = "";
  for (byte i = 0; i < bufferSize; i++) {
    result += (buffer[i] < 0x10 ? "0" : "") + String(buffer[i], DEC);
  }
  Serial.print(result);
}

String toString(byte *buffer, byte bufferSize) {
  String result = "";
  for (byte i = 0; i < bufferSize; i++) {
    result += (buffer[i] < 0x10 ? "0" : "") + String(buffer[i], DEC);
  }
  return result;
}

/**
 * Reads the entire card memory and formats it like the MFRC522 DumpInfo example.
 */
void updateCardDump() {
  lastCardDump = "";
  
  // Header
  lastCardDump += "Card UID:";
  for (byte i = 0; i < rfid.uid.size; i++) {
    lastCardDump += (rfid.uid.uidByte[i] < 0x10 ? " 0" : " ") + String(rfid.uid.uidByte[i], HEX);
  }
  lastCardDump += "\nCard SAK: ";
  lastCardDump += (rfid.uid.sak < 0x10 ? "0" : "") + String(rfid.uid.sak, HEX);
  lastCardDump += "\nPICC type: ";
  lastCardDump += rfid.PICC_GetTypeName(rfid.PICC_GetType(rfid.uid.sak));
  lastCardDump += "\n\n";

  byte buffer[18];
  byte size = sizeof(buffer);
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // Determine number of sectors based on card type
  byte numSectors = 16; // Default to MIFARE 1K
  if (piccType == MFRC522::PICC_TYPE_MIFARE_4K) {
    numSectors = 40;
  } else if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI) {
    numSectors = 5;
  }

  for (byte sector = 0; sector < numSectors; sector++) {
    lastCardDump += "Sector ";
    lastCardDump += sector;
    lastCardDump += "\n";
    
    // Calculate block addresses based on sector (1K/Mini vs 4K)
    byte firstBlock = (sector < 32) ? (sector * 4) : (32 * 4 + (sector - 32) * 16);
    byte numBlocks = (sector < 32) ? 4 : 16;

    // Authenticate the sector
    MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, firstBlock, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
      lastCardDump += "  PCD_Authenticate() failed: ";
      lastCardDump += rfid.GetStatusCodeName(status);
      lastCardDump += "\n\n";
      continue;
    }

    // Read each block in the sector
    for (byte block = 0; block < numBlocks; block++) {
      byte blockAddr = firstBlock + block;
      lastCardDump += "  Block ";
      if (blockAddr < 10) lastCardDump += " ";
      lastCardDump += blockAddr;
      lastCardDump += " ";
      
      status = rfid.MIFARE_Read(blockAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        lastCardDump += "MIFARE_Read() failed: ";
        lastCardDump += rfid.GetStatusCodeName(status);
        lastCardDump += "\n";
        continue;
      }

      // Print the 16 bytes of data
      for (byte index = 0; index < 16; index++) {
        lastCardDump += (buffer[index] < 0x10 ? " 0" : " ") + String(buffer[index], HEX);
      }
      
      // Mark the trailer block
      bool isTrailer = false;
      if (sector < 32) {
        if (block == 3) isTrailer = true;
      } else {
        if (block == 15) isTrailer = true;
      }

      if (isTrailer) {
        lastCardDump += "  [Trailer]";
      }
      lastCardDump += "\n";
    }
    lastCardDump += "\n";
  }
}

// --- Web Server Handlers ---
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='3'>"; // Auto-refresh every 3 seconds
  html += "<title>ESP32 RFID Dump</title>";
  html += "<style>";
  html += "body { background-color: #000000; color: #00FF00; font-family: 'Courier New', Courier, monospace; margin: 0; padding: 20px; }";
  html += "h1 { text-align: center; color: #00FF00; text-shadow: 0 0 5px #00FF00; margin-bottom: 5px; font-size: 1.5em; }";
  html += ".subtitle { text-align: center; color: #008800; margin-bottom: 20px; font-size: 0.85em; }";
  html += ".terminal { background-color: #0a0a0a; border: 1px solid #00FF00; border-radius: 5px; padding: 15px; overflow-x: auto; box-shadow: 0 0 10px rgba(0, 255, 0, 0.2); }";
  html += "pre { margin: 0; white-space: pre-wrap; word-wrap: break-word; line-height: 1.4; font-size: 13px; }";
  html += ".status { color: #FFFF00; font-weight: bold; margin-bottom: 10px; border-bottom: 1px dashed #004400; padding-bottom: 5px; }";
  html += "</style></head><body>";
  html += "<h1>ESP32 RFID READER</h1>";
  html += "<div class='subtitle'>WiFi: ESP32_RFID_Reader | Pass: 12345678 | IP: 192.168.4.1</div>";
  html += "<div class='terminal'>";
  html += "<div class='status'>STATUS: " + scanStatus + " | TOTAL SCANS: " + String(counter) + "</div>";
  html += "<pre>" + lastCardDump + "</pre>";
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(resetButtonPin, INPUT_PULLUP);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8x8log.setRedrawMode(1);

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // --- Start WiFi Access Point ---
  Serial.println("Starting WiFi Access Point...");
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  preferences.begin("my-app", false);

  preferences.getBytes("prev2", pre2NuidPICC, 4);
  preferences.getBytes("prev", preNuidPICC, 4);
  preferences.getBytes("now", nuidPICC, 4);
  counter = preferences.getUInt("counter", 0);

  String now = toString(nuidPICC, 4);
  String prev = toString(preNuidPICC, 4);
  String prev2 = toString(pre2NuidPICC, 4);
  
  if (counter == 0) {
    u8x8log.print("RFID Reader\n");
    u8x8log.print("Ready...\n");
  } else if (counter == 1) {
    u8x8log.print("#1: ");
    u8x8log.print(now);
    u8x8log.print("\n");
  } else if (counter == 2) {
    u8x8log.print("#1: ");
    u8x8log.print(prev);
    u8x8log.print("\n");
    u8x8log.print("#2: ");
    u8x8log.print(now);
    u8x8log.print("\n");
  } else {
    u8x8log.print("#");
    u8x8log.print(counter - 2);
    u8x8log.print(": ");
    u8x8log.print(prev2);
    u8x8log.print("\n");
    u8x8log.print("#");
    u8x8log.print(counter - 1);
    u8x8log.print(": ");
    u8x8log.print(prev);
    u8x8log.print("\n");
    u8x8log.print("#");
    u8x8log.print(counter);
    u8x8log.print(": ");
    u8x8log.print(now);
    u8x8log.print("\n");
  }
  
  Serial.println(F("This code scans the MIFARE Classic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void checkResetButton() {
  int buttonState = digitalRead(resetButtonPin);

  if (buttonState == LOW && !isButtonPressed && (millis() - lastClearTime > clearCooldown)) {
    isButtonPressed = true;
    buttonPressTime = millis();
  }

  if (isButtonPressed) {
    if (buttonState == HIGH) {
      isButtonPressed = false;
    }
    else if (millis() - buttonPressTime >= holdDuration) {
      preferences.end();
      preferences.begin("my-app", false);
      preferences.clear();

      counter = 0;
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = 0;
        preNuidPICC[i] = 0;
        pre2NuidPICC[i] = 0;
      }

      digitalWrite(ledPin, HIGH);
      digitalWrite(PIN_BUZZER, HIGH);
      delay(150);
      digitalWrite(ledPin, LOW);
      digitalWrite(PIN_BUZZER, LOW);
      delay(100);
      digitalWrite(ledPin, HIGH);
      digitalWrite(PIN_BUZZER, HIGH);
      delay(150);
      digitalWrite(ledPin, LOW);
      digitalWrite(PIN_BUZZER, LOW);

      u8x8log.print("\f");
      u8x8log.print("NVS CLEARED!\n");
      u8x8log.print("Ready...\n");
      Serial.println("NVS Cleared successfully.");

      isButtonPressed = false;
      lastClearTime = millis();
      
      // Reset web display variables
      scanStatus = "Waiting for a card...";
      lastCardDump = "Waiting for a card...\n\nPlace a MIFARE Classic card on the reader\nto view the full memory dump.";
    }
  }
}

void loop() {
  server.handleClient(); // Handle incoming web browser requests
  checkResetButton();

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    scanStatus = "Unknown tag type (Not MIFARE Classic)";
    lastCardDump = "Error: Tag is not a MIFARE Classic card.\nOnly MIFARE Mini, 1K, and 4K are supported for full memory dump.";
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println(F("A new card has been detected."));

    for (byte i = 0; i < 4; i++) {
      pre2NuidPICC[i] = preNuidPICC[i];
      preNuidPICC[i] = nuidPICC[i];
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    
    scanStatus = "Reading card memory...";
    server.handleClient(); // Keep web server responsive during the read delay
    
    // Generate the full 64-line dump
    updateCardDump();
    
    scanStatus = "Scan complete";
    
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    
    digitalWrite(ledPin, HIGH);
    digitalWrite(PIN_BUZZER, HIGH);
    printDec(rfid.uid.uidByte, rfid.uid.size);
    
    String now = toString(rfid.uid.uidByte, rfid.uid.size);
    String prev = toString(preNuidPICC, 4);
    String prev2 = toString(pre2NuidPICC, 4);

    preferences.putBytes("prev2", pre2NuidPICC, 4);
    preferences.putBytes("prev", preNuidPICC, 4);
    preferences.putBytes("now", rfid.uid.uidByte, 4);
    counter++;
    preferences.putUInt("counter", counter);
    
    u8x8log.print("#");
    u8x8log.print(counter);
    u8x8log.print(": ");
    u8x8log.print(now);
    u8x8log.print("\n");

    delay(100);
    digitalWrite(ledPin, LOW);
    digitalWrite(PIN_BUZZER, LOW);
  } else {
    Serial.println(F("Card read previously."));
    scanStatus = "Card read previously (Tap a new card)";
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}