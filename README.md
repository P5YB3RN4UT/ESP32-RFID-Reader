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

  <i>The ESP32 must be able to read data from RFID cards using an attached RFID reader module, capturing the unique identifier (UID) and other stored information from the card.</i>
  
- <b>Display Card Info on OLED Screen</b>

  <i>Upon successfully reading an RFID card, the ESP32 should display the card's UID on an OLED screen connected to the microcontroller.</i>
  
- <b>Activate Buzzer and LED on RFID Tap</b>

  <i>When an RFID card is tapped and read, the ESP32 should trigger a buzzer and light up an LED to provide audible and visual feedback indicating a successful read operation.</i>
  
- <b>Debouncing</b>

  <i>Introducing a debounce time interval during which repeated taps of the same card are ignored to prevent duplicate reads.</i>
  
- <b>State Management</b>

  <i>Maintaining a record of recently read card UIDs and ensuring that any repeated taps within a predefined time window are disregarded.</i>

- <b>Clear Record</b>

  <i>Be able to clear NVS (non-volatile data storage) on 2 second button depress.</i>

- <b>Web Interface</b>

  <i>Broadcast a local access point to display a retro terminal-style Web UI showing sectors, blocks and trailer markers of RFID card/tag.</i>

## Hardware

1. RFID Cards and/or Tags
2. ESP32 Module
3. Programming Cable
4. RFID-RC522
5. OLED SSD1306
6. LED
7. Buzzer
8. Push Button Switch


## Software

[ArduinoIDE](https://www.arduino.cc/en/software) - For Programming The Device.


## Schematic

<img src="https://www.media-underground.net/images/rfid-schematic.png">


## Operation

1. Power up the device.
2. Connect to WiFi access point "ESP32_RFID_Reader" with the password "12345678".
3. Type URL http://192.168.4.1 in preferred web browser.
4. Scan card/tag - the OLED will display the tag/card UID whilst the Web UI shows sectors, blocks and trailer markers.
5. Scan another tag/card to repeat.
6. Clear card history by depressing push button for 2 seconds.

