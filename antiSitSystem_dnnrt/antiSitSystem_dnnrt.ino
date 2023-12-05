/*
 *  Spresense_gnss_simple.ino - Simplified gnss example application
 *  Copyright 2019-2021 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <Camera.h>
#include <SPI.h>
#include <EEPROM.h>
#include <DNNRT.h>
#include "Adafruit_ILI9341.h"

#include <SDHCI.h>
SDClass theSD;

/* LCD Settings */
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10

// 撮影画像サイズ
#define CAM_IMG_W 96
#define CAM_IMG_H 96
// 推論画像サイズ
#define DNN_IMG_W 96
#define DNN_IMG_H 96
// 推論画像の左上角座標
#define CAM_CLIP_X 16
#define CAM_CLIP_Y 16


#define LINE_THICKNESS 5

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

uint8_t buf[DNN_IMG_W * DNN_IMG_H];

DNNRT dnnrt;
DNNVariable input(DNN_IMG_W *DNN_IMG_H);

static uint8_t const label[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

void CamCB(CamImage img)
{
    static uint32_t last_mills = 0;

    if (!img.isAvailable())
    {
        Serial.println("Image is not available. Try again");
        return;
    }

    DNNVariable output = detectPersonInImage(CamImage img);
    int index = output.maxIndex();
    bool personDetected;
    if(index == 0){
        personDetected = false;
    }else{
        personDetected = true;
    }

    // 画像描画
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    uint16_t *imgBuf = (uint16_t *)img.getImgBuff();
    tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 320, 224);

    // 処理時間の測定と表示
    uint32_t current_mills = millis();
    duration = (double)(current_mills - last_mills) / 1000;
    Serial.println("duration = " + String(duration));
    last_mills = current_mills;
    if(display_mode == "main"){
        time_from_start += duration;
        Serial.println("time = " + String(time_from_start));
    }
}

DNNVariable detectPersonInImage(CamImage img){
    //YUVをRGBに変更
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    uint16_t *tmp = (uint16_t *)img.getImgBuff();
    //RGBのうちGを抽出
    float *dnnbuf = input.data();
    for (int n = 0; n < DNN_IMG_H * DNN_IMG_W; ++n)
    {
        dnnbuf[n] = (float)((tmp[n] & 0x07E0) >> 5);
    }
    // 推論処理
    dnnrt.inputVariable(input, 0);
    dnnrt.forward();
    DNNVariable output = dnnrt.outputVariable(0);

    return output;
}

void setup()
{
    Serial.begin(115200);

    tft.begin();
    tft.setRotation(3);

    while (!theSD.begin())
    {
        Serial.println("insert sd card");
    }

    File nnbfile = theSD.open("model.nnb");
    int ret = dnnrt.begin(nnbfile);
    if (ret < 0)
    {
        Serial.println("dnnrt.begin failed");
        return;
    }

    theCamera.begin();
    theCamera.startStreaming(true, CamCB);
}

void loop() {}