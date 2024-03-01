#define FASTLED_INTERNAL
#include "M5Dial.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN_A 13
#define NUMPIXELS_A 15
Adafruit_NeoPixel pixels_A(NUMPIXELS_A, PIN_A, NEO_GRB + NEO_KHZ800);
#define DELAYVAL_A 50

#include "BLEDevice.h"
#define T_PERIOD 10
#define S_PERIOD 10

BLEAdvertising *pAdvertising;
uint8_t seq = 0;
uint8_t dispColor = 0;

void setAdvData(BLEAdvertising *pAdvertising) {
  uint16_t dialNum = (uint16_t)(M5Dial.Encoder.read());
  Serial.println(dialNum);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x06);

  std::string strServiceData = "";
  strServiceData += (char)0x08;
  strServiceData += (char)0xff;
  strServiceData += (char)0xff;
  strServiceData += (char)0xff;
  strServiceData += (char)seq;
  strServiceData += (char)(dialNum & 0xff);
  strServiceData += (char)((dialNum >> 8) & 0xff);
  strServiceData += (char)(dispColor & 0xff);
  strServiceData += (char)((dispColor >> 8) & 0xff);

  Serial.print("strServiceData: ");
  for (unsigned int i = 0; i < strServiceData.length(); i++) {
    Serial.print("0x");
    if ((uint8_t)strServiceData[i] < 16) {
      Serial.print("0");
    }
    Serial.print((uint8_t)strServiceData[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  oAdvertisementData.addData(strServiceData);
  pAdvertising->setAdvertisementData(oAdvertisementData);
}


void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  M5Dial.begin(cfg, true, false);
  M5Dial.Display.setTextColor(GREEN);
  M5Dial.Display.setTextDatum(middle_center);
  M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
  M5Dial.Display.setTextSize(2);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  pixels_A.begin();
  pixels_A.clear();
  pixels_A.show();

  BLEDevice::init("M5stack Env");
  BLEServer *pServer = BLEDevice::createServer();
  pAdvertising = pServer->getAdvertising();
}

long oldPosition = -999;
int light = 255;
int brightness = 50;


int range = 1; 
int pixelRange = 0;
int pixelCol[] = { 0, 0, 0 };

int itemSelect = 0;
int prev_x = -1;
int prev_y = -1;
static m5::touch_state_t prev_state;

void loop() {

  M5Dial.update();
  auto disp = M5Dial.Touch.getDetail();


  if (prev_state != disp.state) {
    prev_state = disp.state;
    static constexpr const char *state_name[16] = {
      "none", "touch", "touch_end", "touch_begin",
      "___", "hold", "hold_end", "hold_begin",
      "___", "flick", "flick_end", "flick_begin",
      "___", "drag", "drag_end", "drag_begin"
    };

    if (state_name[disp.state] == "touch_end") {
      dispColor++;
      Serial.println("colorChanged");
      if (dispColor == 4) dispColor = 0;
      colorDisplay();
    }
  }


  long newPosition = M5Dial.Encoder.read();

  if (newPosition != oldPosition) {
    M5Dial.Speaker.tone(8000, 20);
    colorDisplay();
    oldPosition = newPosition;
    Serial.println(newPosition);
  }

  M5.Lcd.setRotation(2);
  M5Dial.Display.drawString(String(newPosition),
                            M5Dial.Display.width() / 2,
                            M5Dial.Display.height() / 2);


  if (newPosition % range == 0) {
    pixels_A.setBrightness(brightness);
    pixels_A.clear();
    pixelRange = int(newPosition / range);
    for (int i = 0; i < pixelRange; i++) {
      pixels_A.setPixelColor(i, pixels_A.Color(pixelCol[0], pixelCol[1], pixelCol[2]));
      pixels_A.show();
    }
  }

  if (M5Dial.BtnA.wasPressed()) {

    setAdvData(pAdvertising);

    for (int i = 0; i < NUMPIXELS_A; i++) {  // 光の玉が流れていくビジュアル
      pixels_A.setPixelColor(i, pixels_A.Color(0, 0, 0));
      pixels_A.setPixelColor(i + pixelRange, pixels_A.Color(pixelCol[0], pixelCol[1], pixelCol[2]));
      pixels_A.show();
      delay(DELAYVAL_A);
    }

    pAdvertising->start();
    delay(T_PERIOD * 100);
    pAdvertising->stop();
    delay(S_PERIOD * 1);
    seq++;

    M5Dial.Encoder.readAndReset();
    pixels_A.clear();
    pixels_A.show();
  }
}


void colorDisplay() {
  switch (dispColor) {
    case 1:
      M5.Lcd.fillScreen(M5.Lcd.color565(light, 0, 0));
      pixelCol[0] = light;
      pixelCol[1] = 0;
      pixelCol[2] = 0;
      break;
    case 2:
      M5.Lcd.fillScreen(M5.Lcd.color565(0, light - 150, 0));
      pixelCol[0] = 0;
      pixelCol[1] = light;
      pixelCol[2] = 0;
      break;
    case 3:
      M5.Lcd.fillScreen(M5.Lcd.color565(0, 0, light));
      pixelCol[0] = 0;
      pixelCol[1] = 0;
      pixelCol[2] = light;
      break;
    default:
      M5.Lcd.fillScreen(M5.Lcd.color565(light, light, light));
      pixelCol[0] = light;
      pixelCol[1] = light;
      pixelCol[2] = light;
      ;
  }
}