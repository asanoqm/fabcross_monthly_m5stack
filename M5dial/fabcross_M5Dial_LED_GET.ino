#define FASTLED_INTERNAL
#include <M5Stack.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN 21
#define NUMPIXELS 75
#define DELAYVAL 20
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int range = 1;
int pixelRange = 0;
int pixelCol[] = { 0, 0, 0 };
int light;
int dialNum;
int dispColor;

#include "BLEDevice.h"
uint8_t seq = 0xFF;
#define MyManufactureId 0xffff
BLEScan* pBLEScan;


void setup() {
  M5.begin();
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(false);

  pixels.begin();
}

void loop() {
  bool found = false;

  BLEScanResults foundDevices = pBLEScan->start(3);
  int count = foundDevices.getCount();
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice d = foundDevices.getDevice(i);
    if (d.haveManufacturerData()) {
      std::string data = d.getManufacturerData();
      int manu = data[1] << 8 | data[0];
      if (manu == MyManufactureId && seq != data[2]) {
        found = true;
        seq = data[2];
        dialNum = (int)(data[4] << 8 | data[3]);
        dispColor = (int)(data[6] << 8 | data[5]);
        //M5.Lcd.printf("Data: %02x %02x %02x %02x %02x %02x %02x\r\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        if (dialNum < 0) {
          light = 0;
        } else if (dialNum > 100) {
          light = 255;
        } else {
          light = dialNum * 255 / 10;
        }
        pixels.clear();
        ledTape();
      }
    }
  }

  for (int i = 0; i < dialNum * 5; i++) {
    if (dialNum == 0) {
      pixels.setBrightness(0);
    } else {
      pixels.setBrightness(255);
    }
    pixels.setPixelColor(i, pixels.Color(pixelCol[0], pixelCol[1], pixelCol[2]));
    pixels.show();
    delay(DELAYVAL);
  }
}


void ledTape() {
  switch (dispColor) {
    case 1:
      M5.Lcd.fillScreen(M5.Lcd.color565(100, 0, 0));
      pixelCol[0] = light;
      pixelCol[1] = 0;
      pixelCol[2] = 0;
      break;
    case 2:
      M5.Lcd.fillScreen(M5.Lcd.color565(0, 100, 0));
      pixelCol[0] = 0;
      pixelCol[1] = light;
      pixelCol[2] = 0;
      break;
    case 3:
      M5.Lcd.fillScreen(M5.Lcd.color565(0, 0, 100));
      pixelCol[0] = 0;
      pixelCol[1] = 0;
      pixelCol[2] = light;
      break;
    default:
      M5.Lcd.fillScreen(M5.Lcd.color565(100, 100, 100));
      pixelCol[0] = light;
      pixelCol[1] = light;
      pixelCol[2] = light;
      ;
  }
}