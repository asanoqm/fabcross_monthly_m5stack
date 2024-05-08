/**
 * @file main.cpp
 * @author Forairaaaaa
 * @brief
 * @version 0.1
 * @date 2023-10-31
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <mooncake.h>
#include <Arduino.h>
#include "hal/hal.h"
#include "servers/servers.h"
#include <esp32cam.h> // esp32camのライブラリを追加

// esp32camのピン割り当て
esp32cam::Resolution initialResolution;
constexpr esp32cam::Pins UnitCamS3{
    D0 : 6,
    D1 : 15,
    D2 : 16,
    D3 : 7,
    D4 : 5,
    D5 : 10,
    D6 : 4,
    D7 : 13,
    XCLK : 11,
    PCLK : 12,
    VSYNC : 42,
    HREF : 18,
    SDA : 17,
    SCL : 41,
    RESET : 21,
    PWDN : -1,
};

void setup()
{
    // Init
    HAL::hal::GetHal()->init();

    // Start server
    if (HAL::hal::GetHal()->getConfig().start_poster == "yes")
        UserDemoServers::start_poster_server();
    else if (HAL::hal::GetHal()->getConfig().start_shooter == "yes")
        UserDemoServers::start_shooter_server();
    else
        UserDemoServers::start_ap_server();
}

void loop()
{
    delay(60000);

    // 画像データを取得
    auto frame = esp32cam::capture();
    uint8_t *image_data = frame->data();
    size_t image_size = frame->size();

    // SDカードに画像を保存する処理
    if (HAL::hal::GetHal()->isSdCardValid())
        if (HAL::hal::GetHal()->saveImage(image_data, image_size))
            Serial.println("Image saved successfully!");
        else
            Serial.println("Failed to save Image.");
    else
        Serial.println("SD card is invalid.");
}
