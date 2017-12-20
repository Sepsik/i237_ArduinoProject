# Arduino Mega RFID-RC522 wiring

## Introduction

**NB! RFID-RC522 requires only 3.3 V DC input voltage. Never connect it to 5V input.**

<div class=pagebreak></div>

## Wiring illustration

![arduino-mega-rfid-rc522-wiring.png](arduino-mega-rfid-rc522-wiring.png)

## Wiring table

| Signal | ATMega2560 port and pin | Arduino Mega 2560 pin | 5V to 3V converter | RFID-RC522 | Wire colour in illustration |
| --- | --- | --- | --- | --- |  --- |
| Slave select | PORTB 0 | 53 | - | SDA | White |
| SPI clock | PORTB 1 | 52 | - | SCK | Orange |
| Master out slave in | PORTB 2 | 51 | - | MOSI | Green |
| Master in slave out | PORTB 3 | 50 | - | MISO | Yellow |
| RF522 reset | PORTL 0 | 49 | - | RST | Brown |
| Ground | GND | GND | GND | GND | Black |
| 5 V DC | - | 5V | VIN | - | Red |
| 3,3 V DC | - | - | VOUT | 3.3 V | Red |

