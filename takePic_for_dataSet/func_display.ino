//LCDモニタ関連の関数

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);
uint16_t disp[width*20*2];
uint8_t buf_bw[width*height];


void setup_display() {
  tft.begin(); 
  tft.setRotation(3);  
  tft.writeFillRect(0, 0, width*2, height*2, 0xFFFF);
}

void display_image(uint16_t* buf) {
  
  for(int z=0; z < height/10; ++z){
    int n = 0; 
    for (int y = z*10; y < z*10 + 10; ++y) {
      for (int x = width; x > 0; --x) {
        // YUV422形式の画像データから輝度成分を抽出しRGB565へ変換
        uint16_t value = buf[y*width + x];
        uint16_t y_h = (value & 0xf000) >> 8;
        uint16_t y_l = (value & 0x00f0) >> 4;
        value = (y_h | y_l);
        uint16_t value6 = (value >> 2);
        uint16_t value5 = (value >> 3);
        // 撮影画像表示部分
        buf_bw[n] = (value5 << 11) | (value6 << 5) | value5;
        disp[n] = (value5 << 11) | (value6 << 5) | value5;
        disp[n+1] = (value5 << 11) | (value6 << 5) | value5;
        disp[width*2+n] = (value5 << 11) | (value6 << 5) | value5;
        disp[width*2+n+1] = (value5 << 11) | (value6 << 5) | value5;
        if (x == offset_x || x == width - offset_x || y == offset_y || y == height - offset_y){
          // 撮影範囲表示部分
          disp[n] = ILI9341_RED;
        }
        // 画像データの読み出し番号更新
        n = n + 2;
        if (n % (width*2) == 0){
          n = n + width*2;
        }
      }
    }
    // 画像の表示
    tft.drawRGBBitmap(0, z*20, disp, width*2, 20); 
  }
}
