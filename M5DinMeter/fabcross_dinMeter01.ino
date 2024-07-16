#include "M5GFX.h"
#include "M5Unified.h"
#include "M5DinMeter.h"

M5GFX display;
M5Canvas canvasA(&display);

void setup() {
  auto cfg = M5.config();     
  DinMeter.begin(cfg, true);  // DinMeterの機能を使うために必要
  display.begin();            
  display.setRotation(1);     
  display.fillScreen(TFT_BLACK);
  canvasA.createSprite(display.width(), display.height());
}

int dispMode = 0;  // モード切り替え用のフラグ
long oldPrice = 2530;
float price = 2530;

void loop() {
  DinMeter.update();  // DinMeterの機能を使うために必要

  // 販売価格を変更する
  if (dispMode == 1) {
    long newPrice = DinMeter.Encoder.read();
    if (newPrice != oldPrice) {
      DinMeter.Speaker.tone(8000, 20);
      oldPrice = newPrice;
      price = newPrice;
    }
  }

  // 販売価格を表示するスプライトを作る
  canvasA.fillRect(0, 0, display.width(), display.height(), BLACK);
  canvasA.setTextColor(WHITE);
  canvasA.setTextSize(0.6);
  canvasA.setFont(&fonts::lgfxJapanGothicP_40);
  canvasA.setCursor(0, 40);
  canvasA.println("書籍「京島の十月」");
  canvasA.printf("Price: %.0fyen", price);
  if (dispMode == 1) {
    canvasA.printf(" ←");
  }

  // ノブを押すとモードが変わる
  if (DinMeter.BtnA.wasPressed()) {
    dispMode = (dispMode + 1) % 2;
    DinMeter.Encoder.write(price);
  }
  
  canvasA.pushSprite(&display, 20, 0);
  delay(10);
}
