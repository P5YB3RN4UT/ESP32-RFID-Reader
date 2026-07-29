<p align="center">
 <img src="https://www.media-underground.net/images/rfid.png">
</p>

<h3 align="center">ESP32 RFID READER</h3>

<div align="center">

[![Status](https://img.shields.io/badge/status-active-success.svg)]()
[![GitHub Pull Requests](https://img.shields.io/github/issues-pr/kylelobo/The-Documentation-Compendium.svg)](https://github.com/hathai25/esp32-rfid-reader/pulls)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

</div>

---

<p align="center"> 
This project involves creating a device that uses an ESP32 microcontroller to read RFID tags.
    <br> 
</p>

## Introduction

The project includes source code for an RFID Reader with the following requirements:

- <b>Read RFID Card/Tag</b>

  <i>The ESP32 must be able to read data from RFID tags and cards using an attached RFID reader module, capturing the unique identifier (UID) and other stored information from the card.</i>
  
- <b>Display Card Info on OLED Screen</b>

  <i>Upon successfully reading an RFID card, the ESP32 should display the card's UID on an OLED screen connected to the microcontroller.</i>

- <b>RGB LED</b>

  <i> The RGB in an idle state should cycle through the full colour spectrum whilst breathing.</i>
  
- <b>Activate Buzzer and RGB LED on RFID Tap</b>

  <i>When an RFID tag or card is tapped and read, the ESP32 should trigger the buzzer twice and simultaneously flash the RGB LED blue to provide audible and visual feedback indicating a successful read operation.</i>
  
- <b>Debouncing</b>

  <i>Introducing a debounce time interval during which repeated taps of the same tag or card are ignored to prevent duplicate reads.</i>
  
- <b>State Management</b>

  <i>Maintaining a record of recently read tag or card UIDs and ensuring that any repeated taps within a predefined time window are disregarded.</i>

- <b>Clear Record</b>

  <i>Be able to clear NVS (non-volatile data storage) on a 2 second button depress. The buzzer should beep exactly five times and simultaneously flash the RGB LED red.</i>

- <b>Web Interface</b>

  <i>Broadcast a local access point to display a retro terminal-style Web UI showing sectors, blocks and trailer markers of a recently scanned RFID tag or card.</i>

## Hardware

1. RFID Tags/Cards
2. ESP32 Module
3. Programming Cable
4. RFID-RC522
5. OLED SSD1306
6. RGB LED
7. 3 x 220Ω Resistors
8. Buzzer
9. Push Button Switch


## Software

[ArduinoIDE](https://www.arduino.cc/en/software) - For Programming The Device.


## Pinout

| 0.96" OLED DISPLAY | ESP32 DEVKIT V1 |
|:---:|:---:|
| GND | GND |
| VCC	| 3V3 |
| SCL	| GPIO22 |
| SDA	| GPIO21 |
	
| RFID-RC522 | ESP32 DEVKIT V1 |
|:---:|:---:|
| GND | GND |
| 3.3V	| 3V3 |
| RST	| GPIO2 |
| MISO	| GPIO19 |
| MOSI	| GPIO23 |
| SCK	| GPIO18 |
|SS/SDA	| GPIO5 |
	
| BUZZER | ESP32 DEVKIT V1 |
|:---:|:---:|
| +	| GPIO4 |
| -	| GND |
	
| RGB LED | ESP32 DEVKIT V1 |
|:---:|:---:|
| RED ➜ 220Ω RESISTOR	| GPIO13 |
| GREEN ➜ 220Ω RESISTOR | GPIO14 |
| BLUE ➜ 220Ω RESISTOR |	GPIO27 |
| COMMON	| GND |
	
| PUSH BUTTON	| ESP32 DEVKIT V1 |
|:---:|:---:|
| 1	| GPIO32 |
| 2	| GND |



## Operation

1. Power up the device.
2. Connect to WiFi access point "ESP32_RFID_Reader" with the password "12345678".
3. Type URL http://192.168.4.1 in preferred web browser.
4. Scan tag or card - the OLED will display the UID whilst the Web UI shows sectors, blocks and trailer markers.
5. Scan another tag or card to repeat.
6. Clear scan history by depressing push button for 2 seconds.

