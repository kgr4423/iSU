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
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>
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
#define IMG_DRAW_OFFSET_X 16
#define IMG_DRAW_OFFSET_Y 16

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

uint8_t buf[DNN_IMG_W * DNN_IMG_H];

DNNRT dnnrt;
DNNVariable input(DNN_IMG_W *DNN_IMG_H);

static uint8_t const label[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

void CamCB(CamImage img)
{
    static uint32_t last_mills = 0;

    // 画像が正常に読み込めたか判定
    if (!img.isAvailable())
    {
        Serial.println("Image is not available. Try again");
        return;
    }

    // 推論処理と人の有無判別
    DNNVariable output = detectPersonInImage(CamImage img);
    

    // カウンタの更新
    double d = sitcountUpdater(personDetected);
    sitCount += d;
    // モードの更新
    determineMode(safe_width, attention_width, danger_width);

    // 画像描画
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    uint16_t *imgBuf = (uint16_t *)img.getImgBuff();
    tft.drawRGBBitmap(IMG_DRAW_OFFSET_X, IMG_DRAW_OFFSET_Y, (uint16_t *)img.getImgBuff(), CAM_IMG_W, CAM_IMG_H);

    // 警告処理
    alert();

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



// 推論処理
bool detectPersonInImage(CamImage img){
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
    int index = output.maxIndex();

    bool personDetected;
    if(index == 0){
        personDetected = false;
    }else{
        personDetected = true;
    }

    return personDetected;
}


// セットアップとループ
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

// LCDモニタ関連の関数


uint16_t disp[160 * 40];

void setup_display()
{
    tft.begin();
    tft.setRotation(3);
    tft.fillRect(0, 0, 320, 240, 0xFFFF);
}

void resetRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c)
{
    tft.fillRect(x, y, w, h, c);
}

void displayText()
{
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.setCursor(175, 54);
    tft.print(" #Person");
    tft.setCursor(175, 72);
    tft.print(" True :");
    tft.print(person_score);
    tft.setCursor(175, 90);
    tft.print(" False:");
    tft.print(no_person_score);
    tft.setCursor(175, 126);
    tft.print(" count:");
    tft.print(sitCount);
    tft.setCursor(175, 144);
    tft.print("  mode:");
    tft.print(process_mode);
}

void display_main(uint16_t *buf, bool result)
{
    // 警告枠表示
    uint16_t pixel_color;
    if(process_mode == 0 || process_mode == 1){
        pixel_color = ILI9341_BLACK;
    }else if(process_mode == 2){
        pixel_color = ILI9341_YELLOW;
    }else{
        pixel_color = ILI9341_RED;
    }
    tft.fillRect(0, 0, 224, 16, pixel_color);
    tft.fillRect(0, 0, 16, 240, pixel_color);
    tft.fillRect(0, 224, 224, 16, pixel_color);
    tft.fillRect(208, 0, 16, 240, pixel_color);
    //タイムバー
    display_timebar();
    //ボタン表示
    switch_box();
    tft.setTextColor(ILI9341_BLACK);
    setText("SETT ", 267, 198, 2);
    setText("-ING ", 267, 213, 2);
    //画像表示
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            //ピクセル
            //左右反転のため「width*2 - 」としている
            int LU = width*2 - x*2;
            int RU = width*2 - x*2 + 1;
            int LD = width*2 - x*2     + width*2;
            int RD = width*2 - x*2 + 1 + width*2;
            if (result && (10 <= y) && (y <= 20) && (10 <= x) && (x <= 20))
            {
                // 判定アイコン表示部分
                disp[LU] = ILI9341_RED; disp[RU] = ILI9341_RED;
                disp[LD] = ILI9341_RED; disp[RD] = ILI9341_RED;
            }
        }
    }
}

void display_setting(){
    resetRect(0, 70, 60, 50, 0xFFFF);
    resetRect(180, 70, 30, 50, 0xFFFF);
    //タイムバー
    display_timebar();
    //ボタン表示
    switch_box();
    tft.setTextColor(ILI9341_BLACK);
    setText(" > ", 270, 105, 2);
    setText(" < ", 270, 155, 2);
    setText("END ", 270, 205, 2);
    //設定画面表示
    setText("TIME SETTING", 5, 16, 3);
    setText(" Set 'safe-time'.", 10, 50, 2);
    tft.setTextSize(3);
    tft.setCursor(65, 90);
    tft.print(safe_time_min);
    setText("   min", 65, 90, 3);
    setText("<", 30, 90, 3);
    setText(">", 190, 90, 3);
    //パラメータ表示
    tft.fillRect(5, 140, 50, 26, ILI9341_GREEN);
    tft.fillRect(5, 170, 50, 26, ILI9341_YELLOW);
    tft.fillRect(5, 200, 50, 26, ILI9341_RED);
    setText("safe", 65, 140, 2);
    setText("atten", 65, 165, 2);
    setText("-tion", 65, 185, 2);
    setText("danger", 65, 205, 2);
    setText("min", 180, 140, 2);
    setText("min", 180, 175, 2);
    setText("min", 180, 205, 2);

    int attention_time_min = (int)(safe_time_min * 0.25 / 0.7);
    int danger_time_min = (int)(safe_time_min * 0.05 / 0.7);
    tft.setTextSize(2);
    tft.setCursor(145, 140);
    tft.print(safe_time_min);
    tft.setCursor(145, 175);
    tft.print(attention_time_min);
    tft.setCursor(145, 205);
    tft.print(danger_time_min);

}

void display_timebar(){
    int all_time_sec = safe_time_sec / 0.7;
    uint16_t now_icon_pos = 240 * sitCount / all_time_sec;
    tft.fillRect(224, 0, 32, 168, ILI9341_GREEN);
    tft.fillRect(224, 168, 32, 60, ILI9341_YELLOW);
    tft.fillRect(224, 228, 32, 12, ILI9341_RED);
    tft.setTextSize(1);
    tft.setCursor(224, now_icon_pos);
    tft.print(" now");
}
void switch_box(){
    tft.drawRect(257, 40, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 90, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 140, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 190, 62, 45, ILI9341_BLACK);
}
void setText(char* text, uint16_t x, uint16_t y, uint8_t size){
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}
