//LCDモニタ関連の関数

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);
uint16_t disp[target_w*target_h];

/* indicator box */
int box_sx = 80;
int box_ex = 90;
int box_sy = 5;
int box_ey = 15;

void setup_display() {
  tft.begin(); 
  tft.setRotation(3);  
}

void resetTextArea(){
  tft.writeFillRect(160, 0, 170, 240, 0xFFFF);
}

void displayText(){
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(175, 72);
  tft.print("person:");
  tft.println(person_score);
  tft.setCursor(175, 90);
  tft.print("    no:");
  tft.println(no_person_score);
  tft.setCursor(175, 126);
  tft.print(" count:");
  tft.println(sitCount);
  tft.setCursor(175, 144);
  tft.print("  mode:");
  tft.println(mode);
}

void display_image(uint16_t* buf, int offset_x, int offset_y
              , int target_w, int target_h, bool result) {
  int n = 0; 
  for (int y = offset_y; y < offset_y + target_h; ++y) {
    for (int x = offset_x; x < offset_x + target_w; ++x) {
      uint16_t value = buf[y*width + x];
      uint16_t y_h = (value & 0xf000) >> 8;
      uint16_t y_l = (value & 0x00f0) >> 4;
      value = (y_h | y_l);       
      uint16_t value6 = (value >> 2);
      uint16_t value5 = (value >> 3);
      disp[n] = (value5 << 11) | (value6 << 5) | value5;
      if (result && (y >= (offset_y + box_sy)) && (y <= (offset_y + box_ey)) 
        && (x >= (offset_x + box_sx)) && (x <= (offset_x + box_ex))) {
        disp[n] = ILI9341_RED;
      }
      ++n;
    }
  }
  tft.drawRGBBitmap(32, 72, disp, target_w, target_h); 
}

// YUV422からRGBへの変換関数(うまくいかないので保留)
// uint16_t* YUV422toRGB(const uint16_t* yuv422Data, int width, int height) {
//   uint16_t* rgbData;
//   for (int y = 0; y < height; ++y) {
//     for (int x = 0; x < width; ++x) {
//       uint16_t value = yuv422Data[y*width + x];
//       uint16_t y_h = (value & 0xf000) >> 8;
//       uint16_t y_l = (value & 0x00f0) >> 4;
//       uint16_t y = (y_h | y_l);       
//       uint16_t u = (value & 0x000f);
//       uint16_t v = (value & 0x0f00) >> 8;
//       int r = (int)y + 1.13983 * ((int)v - 128);
//       int g = (int)y - 0.39465 * ((int)u - 128) - 0.58060 * ((int)v - 128);
//       int b = (int)y + 2.03211 * ((int)u - 128);
//       rgbData[width*y + x] = (r << 11) | (g << 5) | b; 
//     }
//   }
//   return rgbData;
// }
