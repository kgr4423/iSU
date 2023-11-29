//LCDモニタ関連の関数

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);
uint16_t disp[160*40];

void setup_display() {
  tft.begin(); 
  tft.setRotation(3);  
  tft.writeFillRect(0, 0, 320, 240, 0xFFFF);
}

void resetTextArea(){
  tft.writeFillRect(160, 0, 170, 240, 0xFFFF);
}

void displayText(){
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
  tft.print(mode);
}

void display_image(uint16_t* buf, bool result) {
  
  for(int z=0; z < 12; ++z){
    int n = 0; 
    for (int y = z*10; y < z*10 + 10; ++y) {
      for (int x = 160; x > 0; --x) {
        // YUV422形式の画像データから輝度成分を抽出しRGB565へ変換
        uint16_t value = buf[y*160 + x];
        uint16_t y_h = (value & 0xf000) >> 8;
        uint16_t y_l = (value & 0x00f0) >> 4;
        value = (y_h | y_l);
        uint16_t value6 = (value >> 2);
        uint16_t value5 = (value >> 3);
        // 撮影画像表示部分
        disp[n] = (value5 << 11) | (value6 << 5) | value5;
        disp[n+1] = (value5 << 11) | (value6 << 5) | value5;
        disp[320+n] = (value5 << 11) | (value6 << 5) | value5;
        disp[320+n+1] = (value5 << 11) | (value6 << 5) | value5;
        if (result && (10 <= y) && (y <= 20) && (110 <= x) && (x <= 130)){
          //判定アイコン表示部分
          disp[n] = ILI9341_RED;
          disp[n+1] = ILI9341_RED;
          disp[320+n] = ILI9341_RED;
          disp[320+n+1] = ILI9341_RED;
        }
        if (x == offset_x || x == width - offset_x || y == offset_y || y == height - offset_y){
          // 撮影範囲表示部分
          disp[n] = ILI9341_RED;
        }
        if (mode == 3 && (x <= 5 || 155 <= x || y <= 5 || 115 <= y)){
          // 警告枠表示部分
          disp[n] = ILI9341_RED;
          disp[n+1] = ILI9341_RED;
          disp[320+n] = ILI9341_RED;
          disp[320+n+1] = ILI9341_RED;
        }
        // 画像データの読み出し番号更新
        n = n + 2;
        if (n % 320 == 0){
          n = n + 320;
        }
      }
    }
    // 画像の表示
    tft.drawRGBBitmap(0, z*20, disp, 320, 20); 
  }
}
