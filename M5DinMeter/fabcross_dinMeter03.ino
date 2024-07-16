#include "M5DinMeter.h"
#include "M5GFX.h"
#include "M5Unified.h"
#include "UNIT_SCALES.h"

M5GFX display;
M5Canvas canvasA(&display);
M5Canvas canvasB(&display);
UNIT_SCALES scales;

int dispMode = 0;  // モード切り替え用のフラグ
int dispCount = 0;

void setup() {
  auto cfg = M5.config();      // DinMeterの機能を使うために必要
  DinMeter.begin(cfg, true);   // DinMeterの機能を使うために必要
  display.begin();             // 液晶ディスプレイの初期化
  display.setRotation(1);      // 画面を90度回転させる
  display.setTextWrap(false);  // これが効いてない感じがする？
  display.fillScreen(TFT_BLACK);

  while (!scales.begin(&Wire, 2, 1, DEVICE_DEFAULT_ADDR)) {
    M5.Lcd.print("scales connect error");
    delay(1000);
  }
  scales.setLEDColor(0x001000);  // 16進数のRGBでLEDカラーを指定

  canvasA.createSprite(display.width(), display.height());
  canvasB.createSprite(display.width(), display.height());
}


float unit = 150;
float price = 2530;
long oldPrice = 2530;
long oldPosition = -999;

void loop() {
  DinMeter.update();  // DinMeterの機能を使うために必要

  // 基準の重さを調整する
  if (dispMode == 1) {
    long newPosition = DinMeter.Encoder.read();
    if (newPosition != oldPosition) {
      DinMeter.Speaker.tone(8000, 20);
      oldPosition = newPosition;
      unit = newPosition;
      if (unit < 1) unit = 1;
    }
  }

  // 重さと個数を表示するスプライトを作る
  float weight = scales.getWeight();
  if (isnan(weight)) {
    weight = 1.0;  // weightがNaNだった場合、1.0を代入
  }
  canvasA.fillRect(0, 0, display.width(), 120, BLACK);
  canvasA.setTextColor(WHITE);
  canvasA.setTextSize(0.6);
  canvasA.setFont(&fonts::lgfxJapanGothic_40);
  canvasA.setCursor(0, 0);
  canvasA.printf("Weight: %.0fg\n", weight);
  canvasA.printf("Stock: %dpcs\n", int(weight / unit));
  canvasA.printf("Unit: %.0fg/pcs", unit);
  if (dispMode == 1) {
    canvasA.printf(" ←");
  }

  // 販売価格を変更する
  if (dispMode == 3) {
    long newPrice = DinMeter.Encoder.read();
    if (newPrice != oldPrice) {
      DinMeter.Speaker.tone(8000, 20);
      oldPrice = newPrice;
      price = newPrice;
    }
  }

  // 販売価格を表示するスプライトを作る
  canvasB.fillRect(0, 0, display.width(), 120, BLACK);
  canvasB.setTextColor(RED);
  canvasB.setTextSize(0.6);
  canvasB.setFont(&fonts::lgfxJapanGothic_40);
  canvasB.setCursor(0, 0);
  canvasB.println("");
  canvasB.println("書籍「京島の十月」");
  canvasB.printf("Price: %.0fyen", price);
  if (dispMode == 3) {
    canvasB.printf(" ←");
  }

  // ノブを押すとモードが変わる
  if (DinMeter.BtnA.wasPressed()) {
    dispMode = (dispMode + 1) % 2 + dispCount;  // フラグを<0,1> <2,3>の間で循環する
    Serial.println(dispMode);
  }

  // 長押しすると価格表示モードになる
  if (DinMeter.BtnA.pressedFor(3000)) {
    switch (dispCount) {
      case 0:
        dispCount = 2;
        DinMeter.Speaker.tone(8000, 500);
        DinMeter.Encoder.write(price);
        delay(1000);
        break;

      case 2:
        dispCount = 0;
        DinMeter.Speaker.tone(8000, 500);
        DinMeter.Encoder.write(unit);
        delay(1000);
        break;

      default:
        break;
    }
    dispMode = (dispMode + 1) % 2 + dispCount;
    Serial.println(dispMode);
  }

  // スプライトを表示する
  switch (dispMode) {
    case 0:
    case 1:
      canvasA.pushSprite(&display, 10, 10);
      break;

    case 2:
    case 3:
      canvasB.pushSprite(&display, 10, 10);
      break;

    default:
      display.fillScreen(TFT_BLACK);
      break;
  }

  delay(10);
}
