#include "M5DinMeter.h"
#include "M5GFX.h"
#include "M5Unified.h"
#include "UNIT_SCALES.h"  // 計測ユニットを使うために必要

M5GFX display;
M5Canvas canvasB(&display);
UNIT_SCALES scales;  // 計測ユニットを定義

void setup() {
  auto cfg = M5.config();
  DinMeter.begin(cfg, true);  // DinMeterの機能を使うために必要
  display.begin();
  display.setRotation(1);
  display.fillScreen(TFT_BLACK);
  canvasB.createSprite(display.width(), display.height());

// 計測ユニットとの接続を確認し、成功したらLEDを光らせる
  while (!scales.begin(&Wire, 2, 1, DEVICE_DEFAULT_ADDR)) {  
    DinMeter.Display.print("scales connect error");
    delay(1000);
  }
  scales.setLEDColor(0x001000);
}

float unit = 150; // 重さの単位
int dispMode = 0;  // モード切り替え用のフラグ
long oldPosition = -999;

void loop() {
  DinMeter.update();

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
  float weight = scales.getWeight(); // 現在の重さを計測する
  if (isnan(weight)) {
    weight = 1.0;  // weightがNaNだった場合、1.0を代入
  }
  canvasB.fillRect(0, 0, display.width(), 120, BLACK);
  canvasB.setTextColor(WHITE);
  canvasB.setTextSize(0.6);
  canvasB.setFont(&fonts::lgfxJapanGothic_40);
  canvasB.setCursor(0, 0);
  canvasB.printf("Weight: %.0fg\n", weight); //重さを表示
  canvasB.printf("Stock: %dpcs\n", int(weight / unit)); // 個数を表示
  canvasB.printf("Unit: %.0fg/pcs", unit); //基準の重さを表示
  if (dispMode == 1) {
    canvasB.printf(" ←");
  }

  // ボタンを押すとモードが変わる
  if (DinMeter.BtnA.wasPressed()) {
    dispMode = (dispMode + 1) % 2;
    DinMeter.Encoder.write(unit);
  }

  canvasB.pushSprite(&display, 20, 0);
  delay(10);
}
